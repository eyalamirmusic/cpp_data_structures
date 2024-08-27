#pragma once

#include <memory>

namespace EA::Allocators
{
template <typename T>
std::allocator<T>& getDefaultAllocator()
{
    static std::allocator<T> alloc;
    return alloc;
}

template <typename T>
constexpr T* allocate(size_t n)
{
    return getDefaultAllocator<T>().allocate(n);
}

template <typename T>
constexpr void deallocate(void* p, size_t n)
{
    getDefaultAllocator<T>().deallocate(static_cast<T*>(p), n);
}
} // namespace EA::Allocators
