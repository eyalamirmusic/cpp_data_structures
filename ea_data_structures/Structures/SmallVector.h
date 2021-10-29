#pragma once

#include "Vector.h"

namespace EA::SmallVectorHelpers
{
namespace Detail
{
template <typename T>
inline std::allocator<T>& getAllocator()
{
    static std::allocator<T> alloc;
    return alloc;
}

template <typename T>
constexpr T* allocate(size_t n)
{
    return getAllocator<T>().allocate(n);
}

template <typename T>
constexpr void deallocate(void* p, size_t n)
{
    getAllocator<T>().deallocate(static_cast<T*>(p), n);
}
} // namespace Helpers

template <typename T, size_t MaxSize>
struct Allocator
{
    using value_type = T;

    constexpr Allocator() noexcept = default;

    constexpr Allocator(const Allocator& other)
        : isSmallBuffer(other.isSmallBuffer)
    {
    }

    constexpr Allocator(Allocator&& other) noexcept
        : isSmallBuffer(other.isSmallBuffer)
    {
    }

    constexpr Allocator& operator=(const Allocator& other)
    {
        isSmallBuffer = other.isSmallBuffer;
        return *this;
    }

    constexpr Allocator& operator=(Allocator&& other) noexcept
    {
        isSmallBuffer = other.isSmallBuffer;
    }

    constexpr T* allocate(size_t n)
    {
        if (n <= MaxSize)
        {
            isSmallBuffer = true;
            return reinterpret_cast<T*>(&buffer[n]);
        }

        isSmallBuffer = false;
        return Detail::allocate<T>(n);
    }

    constexpr void deallocate(void* p, size_t n)
    {
        if (n > MaxSize)
            Detail::deallocate<T>(p, n);
    }

    bool operator==(const Allocator& other) const
    {
        return isSmallBuffer == !other.isSmallBuffer;
    }

    std::aligned_storage_t<sizeof(T), alignof(T)> buffer[MaxSize];
    bool isSmallBuffer = false;
};

} // namespace EA::SmallVectorHelpers

namespace EA
{
template <typename T, size_t N>
using SmallVector = Vector<T, SmallVectorHelpers::Allocator<T, N>>;
}
