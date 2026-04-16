#pragma once

#include <cassert>

namespace EA::Allocators::StaticVector
{
//An STL-compatible allocator backed entirely by an in-struct buffer of
//MaxSize elements: allocations above MaxSize throw std::bad_alloc.
//Pair with an STL container to get a container that never touches the heap.
template <typename T, size_t MaxSize>
struct Allocator
{
    using value_type = T;

    template <typename U>
    struct rebind
    {
        using other = Allocator<U, MaxSize>;
    };

    Allocator() = default;

    Allocator(const Allocator&) {}
    Allocator(Allocator&&) noexcept {}

    Allocator& operator=(const Allocator&) { return *this; }

    Allocator& operator=(const Allocator&&) noexcept { return *this; }

    T* allocate(size_t n)
    {
        if (n <= MaxSize)
            return reinterpret_cast<T*>(&buffer[0]);

        //You can't allocate more than pre-specified size
        throw std::bad_alloc();
    }

    static void deallocate(void*, size_t) {}

    bool operator==(const Allocator&) const { return false; }

    std::aligned_storage_t<sizeof(T), alignof(T)> buffer[MaxSize];
};

} // namespace EA::Allocators::StaticVector
