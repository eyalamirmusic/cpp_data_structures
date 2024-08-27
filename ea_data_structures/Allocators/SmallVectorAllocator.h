#pragma once

#include "DefaultAllocators.h"

namespace EA::Allocators::SmallVector
{
struct Base
{
    bool isSmallBuffer = false;
};

template <typename T, size_t MaxSize>
struct Allocator : Base
{
    using value_type = T;

    template <typename U>
    struct rebind
    {
        using other = Allocator<U, MaxSize>;
    };

    Allocator() = default;

    Allocator(const Base& other) { isSmallBuffer = other.isSmallBuffer; }
    Allocator(const Allocator& other) { isSmallBuffer = other.isSmallBuffer; }
    Allocator(Allocator&& other) noexcept { isSmallBuffer = other.isSmallBuffer; }

    Allocator& operator=(const Base& other)
    {
        isSmallBuffer = other.isSmallBuffer;
        return *this;
    }

    Allocator& operator=(Base&& other) noexcept
    {
        isSmallBuffer = other.isSmallBuffer;
        return *this;
    }

    T* allocate(size_t n)
    {
        if (n <= MaxSize)
        {
            isSmallBuffer = true;
            return reinterpret_cast<T*>(&buffer[0]);
        }

        isSmallBuffer = false;
        return Allocators::allocate<T>(n);
    }

    static void deallocate(void* p, size_t n)
    {
        if (n > MaxSize)
            Allocators::deallocate<T>(p, n);
    }

    bool operator==(const Allocator& other) const
    {
        return isSmallBuffer == !other.isSmallBuffer;
    }

    std::aligned_storage_t<sizeof(T), alignof(T)> buffer[MaxSize];
};

} // namespace EA::Allocators::SmallVector