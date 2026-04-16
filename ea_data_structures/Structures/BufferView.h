#pragma once

namespace EA
{
//A non-owning view over a contiguous buffer of T, carrying just a pointer and
//a size. Similar to std::span but with int-based size semantics matching the
//rest of this library.
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

//A view over a 2D buffer represented as `T* const*` (an array of row/channel
//pointers) plus the number of rows and the row length. Iterating yields a
//BufferView per row — intended for accessing multichannel audio buffers.
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