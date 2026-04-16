#pragma once

#include "Vector.h"
#include "BufferView.h"

namespace EA
{
//A 2D container stored as one contiguous Vector<T>: dimensions × dimSize
//elements laid out row-by-row. Indexing with operator[] yields a BufferView
//onto the row. Cache-friendlier than nested vectors for audio/DSP data.
template <typename T>
struct FlatMultiVector
{
    void resize(int dimensions, int sizeToUse) noexcept
    {
        numDimensions = dimensions;
        dimSize = sizeToUse;
        container.resize(numDimensions * dimSize);
    }

    void reserve(int dimensions, int sizeToUse) noexcept
    {
        container.reserve(dimensions * sizeToUse);
    }

    int size() const noexcept { return numDimensions; }

    int getDimensionStart(int dimension) const noexcept
    {
        return dimSize * dimension;
    }

    T* getData(int index) noexcept
    {
        return container.data() + getDimensionStart(index);
    }

    BufferView<T> operator[](int index) noexcept
    {
        return {getData(index), dimSize};
    }

    int numDimensions = 0;
    int dimSize = 0;
    Vector<T> container;
};
} // namespace EA