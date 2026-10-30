#pragma once

#include <ScopedMemoryAllocations/Allocations.h>

#include <cstddef>
#include <memory_resource>
#include <utility>

namespace EA::TestHelpers
{
//True when ScopedMemoryAllocations actually interposes malloc/free and global
//new/delete. On Windows (or with SCOPED_ALLOCATIONS_TEST=OFF) the library is a
//no-op, so "the heap was touched" can't be observed - only "it wasn't".
#ifdef EA_SCOPED_ALLOCATIONS_TEST
constexpr bool heapTrackingEnabled = true;
#else
constexpr bool heapTrackingEnabled = false;
#endif

//Runs func with global heap use banned on this thread and returns how many
//times the global heap was touched while it ran. Every malloc/calloc/realloc/
//free/posix_memalign and global new/delete counts as one call, so a vector
//regrowing its buffer counts twice: the new allocation and the old free.
template <typename Func>
int countGlobalHeapCalls(Func&& func)
{
    auto calls = 0;
    Allocations::setViolationHandler([&calls] { ++calls; });

    {
        auto noAllocs = Allocations::ScopedSetter();
        std::forward<Func>(func)();
    }

    Allocations::setViolationHandler({});
    return calls;
}

//Expected result of countGlobalHeapCalls() for a path that does touch the
//heap: the real count where tracking works, zero where it is compiled out.
constexpr int expectedHeapCalls(int calls)
{
    return heapTrackingEnabled ? calls : 0;
}

//MSVC's debug STL (_ITERATOR_DEBUG_LEVEL > 0) gives every container a
//"container proxy" allocated through the container's own allocator - so
//through its pmr resource. Exact resource allocation counts add this once per
//container constructed on the resource.
#if defined(_ITERATOR_DEBUG_LEVEL) && _ITERATOR_DEBUG_LEVEL > 0
constexpr int proxyAllocationsPerContainer = 1;
#else
constexpr int proxyAllocationsPerContainer = 0;
#endif

//A memory_resource that forwards to an upstream resource and counts the
//calls, so tests can tell which resource an allocation actually went through.
//The upstream is explicit on purpose: std::pmr::new_delete_resource() goes
//through aligned operator new on libstdc++, and ScopedMemoryAllocations'
//replacement of it passes alignments below sizeof(void*) straight to
//posix_memalign, which rejects them with std::bad_alloc.
class CountingResource final : public std::pmr::memory_resource
{
public:
    explicit CountingResource(std::pmr::memory_resource* upstreamToUse)
        : upstream(upstreamToUse)
    {
    }

    //Pointer, size and alignment of a single allocate/deallocate call.
    struct Call
    {
        void* pointer = nullptr;
        std::size_t bytes = 0;
        std::size_t alignment = 0;
    };

    int allocations = 0;
    int deallocations = 0;
    std::size_t bytesAllocated = 0;
    std::size_t bytesDeallocated = 0;
    Call lastAllocation;
    Call lastDeallocation;

    int live() const { return allocations - deallocations; }

private:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override
    {
        ++allocations;
        bytesAllocated += bytes;
        auto* ptr = upstream->allocate(bytes, alignment);
        lastAllocation = {ptr, bytes, alignment};
        return ptr;
    }

    void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override
    {
        ++deallocations;
        bytesDeallocated += bytes;
        lastDeallocation = {ptr, bytes, alignment};
        upstream->deallocate(ptr, bytes, alignment);
    }

    bool do_is_equal(const memory_resource& other) const noexcept override
    {
        return &other == this;
    }

    std::pmr::memory_resource* upstream;
};

//A CountingResource over a fixed stack buffer with no upstream, so anything
//allocated through it provably never reaches the global heap (exhausting the
//buffer throws std::bad_alloc instead of falling back to new).
template <std::size_t Size>
struct StackResource
{
    StackResource() = default;
    StackResource(const StackResource&) = delete;
    StackResource& operator=(const StackResource&) = delete;

    alignas(std::max_align_t) std::byte buffer[Size] {};
    std::pmr::monotonic_buffer_resource arena {
        buffer, Size, std::pmr::null_memory_resource()};
    CountingResource counter {&arena};
};
} // namespace EA::TestHelpers
