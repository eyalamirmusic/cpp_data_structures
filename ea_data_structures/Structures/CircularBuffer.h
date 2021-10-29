#include "Vector.h"

//A simple circular buffer, meant to implement things like an audio delay line
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

    T& operator[](int index) noexcept { return internal[index % internal.size()]; }

    void resize(int size, T defaultValue = T(0))
    {
        internal.clear();
        internal.resize(size);
        internal.fill(defaultValue);
    }

    void reserve(int size) { internal.reserve(size); }

private:
    Vector<T> internal;
};
} // namespace EA
