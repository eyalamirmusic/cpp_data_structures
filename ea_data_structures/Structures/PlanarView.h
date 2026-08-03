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
                             int channelToUse) noexcept
        : ptr(dataToUse)
        , numSamples(numSamplesToUse)
        , channel(channelToUse)
    {
    }

    constexpr Span<T> operator*() const noexcept
    {
        return {ptr + channel * numSamples, numSamples};
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
    int channel = 0;
};

//A non-owning view over a planar (channel-major) block held in one contiguous
//allocation: every sample of channel 0, then every sample of channel 1, and so
//on. Slicing yields a Span per channel, so callers never write
//channel * numSamples by hand.
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

    PlanarView() = default;

    constexpr PlanarView(T* dataToUse,
                         SizeType numChannelsToUse,
                         SizeType numSamplesToUse) noexcept
        : ptr(dataToUse)
        , numChannels(numChannelsToUse.get<int>())
        , numSamples(numSamplesToUse.get<int>())
    {
    }

    //Splits one flat view evenly between the channels
    constexpr PlanarView(Span<T> flatData, SizeType numChannelsToUse) noexcept
        : ptr(flatData.data())
        , numChannels(numChannelsToUse.get<int>())
        , numSamples(numChannels > 0 ? flatData.size() / numChannels : 0)
    {
    }

    //Allows PlanarView<T> -> PlanarView<const T>, but not the other way around
    template <typename U>
        requires(!std::same_as<U, T> && std::is_convertible_v<U (*)[], T (*)[]>)
    constexpr PlanarView(const PlanarView<U>& other) noexcept
        : ptr(other.data())
        , numChannels(other.getNumChannels())
        , numSamples(other.getNumSamples())
    {
    }

    constexpr int getNumChannels() const noexcept { return numChannels; }

    //Samples in a single channel
    constexpr int getNumSamples() const noexcept { return numSamples; }

    //Samples across every channel
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

    constexpr T* data() const noexcept { return ptr; }

    constexpr Span<T> getChannel(SizeType channel) const noexcept
    {
        if (empty())
            return {};

        return {ptr + channel.get<int>() * numSamples, numSamples};
    }

    constexpr Span<T> operator[](SizeType channel) const noexcept
    {
        return getChannel(channel);
    }

    constexpr T* getChannelPointer(SizeType channel) const noexcept
    {
        return getChannel(channel).data();
    }

    //The whole block as a single flat view, still in channel-major order
    constexpr Span<T> flat() const noexcept { return {ptr, getNumElements()}; }

    constexpr Iterator begin() const noexcept { return {ptr, numSamples, 0}; }

    constexpr Iterator end() const noexcept
    {
        return {ptr, numSamples, numChannels};
    }

    void fill(const T& value) const
        requires(!std::is_const_v<T>)
    {
        flat().fill(value);
    }

private:
    T* ptr = nullptr;
    int numChannels = 0;
    int numSamples = 0;
};

template <typename T>
PlanarView(T*, SizeType, SizeType) -> PlanarView<T>;

template <typename T>
PlanarView(Span<T>, SizeType) -> PlanarView<T>;

} // namespace EA
