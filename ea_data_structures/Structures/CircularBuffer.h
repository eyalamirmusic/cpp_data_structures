#pragma once

#include "Vector.h"

//A simple circular buffer, meant to implement things like an audio delay line
namespace CircularAccess
{

template <typename T>
T wrap(T index, T size) noexcept
{
    return (index % size + size) % size;
}
} // namespace CircularAccess

namespace EA
{
template <typename T>
class CircularBuffer
{
public:
    CircularBuffer() = default;
    CircularBuffer(int initialSize, T initialValue = T(0))
    {
        resize(initialSize, initialValue);
    }

    template <typename SizeType>
    T& operator[](SizeType index) noexcept
    {
        auto wrapped = CircularAccess::wrap(index, (SizeType) internal.size());
        return internal[wrapped];
    }

    int size() const noexcept { return internal.size(); }

    void fill(T value = T(0)) { internal.fill(value); }

    void resize(int size, T defaultValue = T(0))
    {
        internal.clear();
        internal.resize(size);
        fill(defaultValue);
    }

    void reserve(int size) { internal.reserve(size); }

private:
    Vector<T> internal;
};

} // namespace EA
