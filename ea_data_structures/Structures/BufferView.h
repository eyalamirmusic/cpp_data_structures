#pragma once

#include "Span.h"

namespace EA
{
//A non-owning view over a contiguous buffer of T. Kept as the name this
//library has always used for a one-dimensional buffer view; Span is the
//implementation
template <typename T>
using BufferView = Span<T>;

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

    Span<T> operator*() { return {*buffer, internalSize}; }

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
//Span per row — intended for accessing multichannel audio buffers.
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