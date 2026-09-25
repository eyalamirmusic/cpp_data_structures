#pragma once

#include "Span.h"

namespace EA
{
//Iterates the channels of a PlanarView, yielding a Span per channel.
//
//The shape is held by value rather than as a pointer back to the view, so an
//iterator stays valid even once the view it came from is gone. That's what
//makes for (auto channel: getBlock().getView()) safe in C++20, where only the
//outermost temporary of the range expression gets its lifetime extended
template <typename T>
class PlanarIterator
{
public:
    constexpr PlanarIterator(T* dataToUse,
                             int numSamplesToUse,
                             int strideToUse,
                             int channelToUse) noexcept
        : ptr(dataToUse)
        , numSamples(numSamplesToUse)
        , stride(strideToUse)
        , channel(channelToUse)
    {
    }

    constexpr Span<T> operator*() const noexcept
    {
        return {ptr + channel * stride, numSamples};
    }

    constexpr PlanarIterator& operator++() noexcept
    {
        ++channel;
        return *this;
    }

    constexpr PlanarIterator& operator--() noexcept
    {
        --channel;
        return *this;
    }

    constexpr bool operator==(const PlanarIterator& other) const noexcept
    {
        return channel == other.channel;
    }

    constexpr bool operator!=(const PlanarIterator& other) const noexcept
    {
        return channel != other.channel;
    }

private:
    T* ptr = nullptr;
    int numSamples = 0;
    int stride = 0;
    int channel = 0;
};

//A non-owning view over a planar (channel-major) block: every sample of
//channel 0, then every sample of channel 1, and so on. Channel c starts
//c * channelStride elements after channel 0. The stride defaults to the
//channel length, making the block one contiguous allocation; a larger stride
//leaves gaps between channels, which is what a sub-view over a range of
//samples looks like. Slicing yields a Span per channel, so callers never write
//channel * stride by hand.
//
//For the other common multichannel layout - an array of per-channel pointers -
//see TwoDimensionalBufferView in BufferView.h
//
//Constness is shallow, exactly like Span: a read-only view is spelled
//PlanarView<const T>, and every accessor is a const member handing back a
//mutable Span<T>
template <typename T>
class PlanarView
{
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = int;
    using Iterator = PlanarIterator<T>;

    //Passed as subView's count to mean "everything up to the end"
    static constexpr int toEnd = Span<T>::toEnd;

    PlanarView() = default;

    constexpr PlanarView(T* dataToUse,
                         SizeType numChannelsToUse,
                         SizeType numSamplesToUse) noexcept
        : ptr(dataToUse)
        , numChannels(numChannelsToUse.get<int>())
        , numSamples(numSamplesToUse.get<int>())
        , channelStride(numSamples)
    {
    }

    //Channel c starts channelStride elements after channel c - 1, which must
    //be at least the channel length so channels never overlap
    constexpr PlanarView(T* dataToUse,
                         SizeType numChannelsToUse,
                         SizeType numSamplesToUse,
                         SizeType channelStrideToUse) noexcept
        : ptr(dataToUse)
        , numChannels(numChannelsToUse.get<int>())
        , numSamples(numSamplesToUse.get<int>())
        , channelStride(channelStrideToUse.get<int>())
    {
        assert(numChannels <= 1 || channelStride >= numSamples);
    }

    //Splits one flat view evenly between the channels
    constexpr PlanarView(Span<T> flatData, SizeType numChannelsToUse) noexcept
        : ptr(flatData.data())
        , numChannels(numChannelsToUse.get<int>())
        , numSamples(numChannels > 0 ? flatData.size() / numChannels : 0)
        , channelStride(numSamples)
    {
    }

    //Allows PlanarView<T> -> PlanarView<const T>, but not the other way around
    template <typename U>
        requires(!std::same_as<U, T> && std::is_convertible_v<U (*)[], T (*)[]>)
    constexpr PlanarView(const PlanarView<U>& other) noexcept
        : ptr(other.data())
        , numChannels(other.getNumChannels())
        , numSamples(other.getNumSamples())
        , channelStride(other.getChannelStride())
    {
    }

    constexpr int getNumChannels() const noexcept { return numChannels; }

    //Samples in a single channel
    constexpr int getNumSamples() const noexcept { return numSamples; }

    //Elements from the start of one channel to the start of the next
    constexpr int getChannelStride() const noexcept { return channelStride; }

    //True when the channels sit back to back with no gaps between them
    constexpr bool isContiguous() const noexcept
    {
        return empty() || numChannels <= 1 || channelStride == numSamples;
    }

    //Samples across every channel - the logical count, excluding any gaps
    constexpr int getNumElements() const noexcept
    {
        if (empty())
            return 0;

        return numChannels * numSamples;
    }

    constexpr bool empty() const noexcept
    {
        return numChannels <= 0 || numSamples <= 0;
    }

    //The first sample of channel 0
    constexpr T* data() const noexcept { return ptr; }

    constexpr Span<T> getChannel(SizeType channel) const noexcept
    {
        if (empty())
            return {};

        return {ptr + channel.get<int>() * channelStride, numSamples};
    }

    constexpr Span<T> operator[](SizeType channel) const noexcept
    {
        return getChannel(channel);
    }

    constexpr T* getChannelPointer(SizeType channel) const noexcept
    {
        return getChannel(channel).data();
    }

    //The samples [offset, offset + count) of every channel, keeping the
    //stride. Both are clamped to the channel length, and a count of toEnd
    //takes everything from offset onwards
    constexpr PlanarView subView(SizeType offset,
                                 SizeType count = toEnd) const noexcept
    {
        auto start = clampToLength(offset.get<int>());
        auto remaining = numSamples - start;
        auto length = count.get<int>();

        if (length < 0 || length > remaining)
            length = remaining;

        return {ptr + start, numChannels, length, channelStride};
    }

    //The whole block as a single flat view, still in channel-major order.
    //Only meaningful when the channels are contiguous
    constexpr Span<T> flat() const noexcept
    {
        assert(isContiguous());
        return {ptr, getNumElements()};
    }

    constexpr Iterator begin() const noexcept
    {
        return {ptr, numSamples, channelStride, 0};
    }

    constexpr Iterator end() const noexcept
    {
        return {ptr, numSamples, channelStride, numChannels};
    }

    //Writes every channel, leaving any gaps between them untouched
    void fill(const T& value) const
        requires(!std::is_const_v<T>)
    {
        for (auto channel: *this)
            channel.fill(value);
    }

private:
    constexpr int clampToLength(int index) const noexcept
    {
        if (index < 0)
            return 0;

        return index > numSamples ? numSamples : index;
    }

    T* ptr = nullptr;
    int numChannels = 0;
    int numSamples = 0;
    int channelStride = 0;
};

template <typename T>
PlanarView(T*, SizeType, SizeType) -> PlanarView<T>;

template <typename T>
PlanarView(T*, SizeType, SizeType, SizeType) -> PlanarView<T>;

template <typename T>
PlanarView(Span<T>, SizeType) -> PlanarView<T>;

} // namespace EA
