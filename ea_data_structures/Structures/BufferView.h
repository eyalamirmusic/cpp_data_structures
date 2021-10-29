#pragma once

namespace EA
{
template <typename T>
struct BufferView
{
    BufferView(T* bufferToUse, int sizeToUse)
        : buffer(bufferToUse)
        , size(sizeToUse)
    {
    }

    T* begin() const { return buffer; }
    T* end() const { return buffer + size; }

    T& operator[](int index) { return buffer[index]; }

    T* buffer;
    int size;
};

template <typename T>
struct TwoDimensionalBufferIterator
{
    TwoDimensionalBufferIterator(T** bufferToUse, int internalSizeToUse)
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
        return buffer == other.start;
    }

    bool operator!=(const TwoDimensionalBufferIterator& other)
    {
        return buffer != other.buffer;
    }

    T** buffer;
    int internalSize;
};

template <typename T>
struct TwoDimensionalBufferView
{
    TwoDimensionalBufferView(T** bufferToUse, int sizeToUse, int internalSizeToUse)
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

    T** buffer;
    int size;
    int internalSize;
};

template <typename T>
TwoDimensionalBufferView<T>
    getViewFor(T** container, int numChannels, int numSamples)
{
    return {container, numChannels, numSamples};
}

} // namespace EA