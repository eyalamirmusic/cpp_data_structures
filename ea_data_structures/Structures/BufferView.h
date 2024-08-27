#pragma once

namespace EA
{
template <typename T>
struct BufferView
{
    BufferView(T* bufferToUse, int sizeToUse)
        : buffer(bufferToUse)
        , bufSize(sizeToUse)
    {
    }

    int size() const noexcept { return bufSize; }

    T* begin() const noexcept { return buffer; }
    T* end() const noexcept { return buffer + bufSize; }

    T& operator[](int index) noexcept { return buffer[index]; }

    T* buffer = nullptr;
    int bufSize = 0;
};

template <typename T>
struct TwoDimensionalBufferIterator
{
    TwoDimensionalBufferIterator(T* const* bufferToUse, int internalSizeToUse)
        : buffer(bufferToUse)
        , internalSize(internalSizeToUse)
    {
    }

    TwoDimensionalBufferIterator& operator++()
    {
        ++buffer;
        return *this;
    }

    TwoDimensionalBufferIterator& operator--()
    {
        --buffer;
        return *this;
    }

    BufferView<T> operator*() { return {*buffer, internalSize}; }

    bool operator==(const TwoDimensionalBufferIterator& other)
    {
        return buffer == other.buffer;
    }

    bool operator!=(const TwoDimensionalBufferIterator& other)
    {
        return buffer != other.buffer;
    }

    T* const* buffer;
    int internalSize;
};

template <typename T>
struct TwoDimensionalBufferView
{
    TwoDimensionalBufferView(T* const* bufferToUse,
                             int sizeToUse,
                             int internalSizeToUse)
        : buffer(bufferToUse)
        , size(sizeToUse)
        , internalSize(internalSizeToUse)
    {
    }

    TwoDimensionalBufferIterator<T> begin() const { return {buffer, internalSize}; }
    TwoDimensionalBufferIterator<T> end() const
    {
        return {buffer + size, internalSize};
    }

    T* const* buffer;
    int size;
    int internalSize;
};

template <typename T>
TwoDimensionalBufferView<T>
    getViewFor(T* const* container, int numChannels, int numSamples)
{
    return {container, numChannels, numSamples};
}

} // namespace EA