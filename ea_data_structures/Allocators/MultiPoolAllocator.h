#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <memory_resource>
#include <stdexcept>
#include <utility>
#include <vector>

namespace EA::Allocators
{
//Alignment that the heap storage behind a DynamicMemory gets from operator new.
//Anything stricter is handled by over-allocating and aligning inside the block.
constexpr size_t defaultAlignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__;

class DynamicMemory
{
public:
    DynamicMemory(size_t size)
    {
        data.reserve(size);
        handedOut = data.data();
    }

    //Bytes a block needs so that `size` bytes aligned to `alignment` fit in it.
    static constexpr size_t getRequiredSpace(size_t size, size_t alignment)
    {
        return alignment > defaultAlignment ? size + alignment - 1 : size;
    }

    void* allocate(size_t size, size_t alignment = defaultAlignment)
    {
        auto space = getRequiredSpace(size, alignment);
        data.resize(space);
        handedOut = data.data();

        if (alignment > defaultAlignment)
            handedOut = std::align(alignment, size, handedOut, space);

        return handedOut;
    }

    //Matches the pointer last returned by allocate(), which for an
    //over-aligned request sits a few bytes into the block.
    bool operator==(const void* other) const { return handedOut == other; }

private:
    std::vector<std::byte> data;
    void* handedOut = nullptr;
};

using SingleResource = std::unique_ptr<DynamicMemory>;

struct ResourceContainer
{
    ResourceContainer(size_t sizeToReserve) { resources.reserve(sizeToReserve); }

    void push_back(SingleResource&& resourceToUse)
    {
        resources.emplace_back(std::move(resourceToUse));
    }

    SingleResource pop_back() { return pop(resources.size() - 1); }

    SingleResource pop(size_t index)
    {
        auto obj = std::move(resources[index]);
        resources.erase(resources.begin() + (int) index);
        return obj;
    }

    SingleResource findAndPop(const void* data)
    {
        for (size_t index = 0; index < resources.size(); ++index)
        {
            if (*resources[index] == data)
                return pop(index);
        }

        throw std::runtime_error("Trying to deallocate a non-existing memory");
    }

    bool empty() const { return resources.empty(); }
    void create(size_t size) { push_back(std::make_unique<DynamicMemory>(size)); }

    std::vector<SingleResource> resources;
};

//A pool of reusable heap blocks: allocate() hands out a free block (or
//creates one if the free list is empty) and deallocate() returns it to the
//free list without actually freeing it. Intended for contexts where you want
//to avoid malloc/free calls in a hot path after an initial warm-up.
//
//Heap-backed, not an arena: every block is its own heap allocation, made up
//front or on demand. The heap is still hit when the free list runs dry or a
//request is bigger than the block it gets (that block regrows).
//
//Pointer stability: a pointer stays valid and never moves until it is
//deallocated (or the pool is destroyed). Only blocks taken from the free list
//are ever resized; a block in use is never touched, and growing the pool only
//moves the unique_ptrs that own blocks between the free and used lists.
class MemoryPool
{
public:
    MemoryPool(int poolObjectSizeToUse,
               int freeListInitialSize = 16,
               int numReservedObjects = 1024)
        : poolObjectSize((size_t) poolObjectSizeToUse)
        , free((size_t) numReservedObjects)
        , used((size_t) numReservedObjects)
    {
        for (int index = 0; index < freeListInitialSize; ++index)
            free.create(poolObjectSize);
    }

    //Alignments above defaultAlignment cost alignment - 1 extra bytes of the
    //block, so a block only fits `size` bytes at that alignment if it holds
    //DynamicMemory::getRequiredSpace(size, alignment); a smaller one regrows.
    void* allocate(size_t size, size_t alignment = defaultAlignment)
    {
        if (free.empty())
        {
            auto space = DynamicMemory::getRequiredSpace(size, alignment);
            free.create(std::max(space, poolObjectSize));
        }

        return allocateFromFreePool(size, alignment);
    }

    void deallocate(const void* data)
    {
        if (auto u = used.findAndPop(data))
            free.push_back(std::move(u));
    }

private:
    void* allocateFromFreePool(size_t size, size_t alignment)
    {
        auto last = free.pop_back();
        auto addr = last->allocate(size, alignment);
        used.push_back(std::move(last));

        return addr;
    }

    void addFreeObject() { free.create(poolObjectSize); }

    size_t poolObjectSize = 0;
    ResourceContainer free;
    ResourceContainer used;
};

namespace Detail
{
//The std::pmr::memory_resource plumbing shared by MemoryPoolResource and
//ObjectPoolResource: every request, including its alignment, goes to a
//MemoryPool.
class PoolResourceBase : public std::pmr::memory_resource
{
protected:
    PoolResourceBase(int objectSize, int freeListInitial, int reservedObjects)
        : pool(objectSize, freeListInitial, reservedObjects)
    {
    }

private:
    void* do_allocate(size_t bytes, size_t alignment) override
    {
        return pool.allocate(bytes, alignment);
    }

    void do_deallocate(void* data, size_t, size_t) override
    {
        pool.deallocate(data);
    }

    bool do_is_equal(const memory_resource& other) const noexcept override
    {
        return &other == this;
    }

    MemoryPool pool;
};
} // namespace Detail

//A std::pmr::memory_resource adapter that routes allocations through a
//MemoryPool. Lets you plug the pool into PMR-aware containers. The defaults
//pre-fill 16 blocks of 64 KB (about 1 MB up front).
//
//Backed by ordinary heap memory, not a stack or static arena: each block is a
//separate heap allocation, and an empty free list or a request bigger than a
//block still reaches the heap. Pointers are stable: what allocate() returns
//never moves until it is deallocated, however the pool grows (see MemoryPool).
class MemoryPoolResource final : public Detail::PoolResourceBase
{
public:
    MemoryPoolResource(int objectSize = 64 * 1024,
                       int freeListInitial = 16,
                       int reservedObjects = 1024)
        : PoolResourceBase(objectSize, freeListInitial, reservedObjects)
    {
    }
};

//A pool resource whose blocks each hold exactly one T, honouring alignof(T),
//pre-filled with numObjects blocks and with bookkeeping reserved for as many,
//so up to numObjects live allocate(sizeof(T), alignof(T)) calls never touch
//the heap. Anything larger than a T - a shared_ptr control block, a list
//node - regrows the block it lands in the first time it uses it.
//Like MemoryPoolResource, each block is its own heap allocation, the heap is
//hit again past numObjects live allocations, and pointers never move until
//they are deallocated.
template <typename T>
class ObjectPoolResource final : public Detail::PoolResourceBase
{
public:
    static constexpr int blockSize =
        (int) DynamicMemory::getRequiredSpace(sizeof(T), alignof(T));

    explicit ObjectPoolResource(int numObjects = 1024)
        : PoolResourceBase(blockSize, numObjects, numObjects)
    {
    }
};
} // namespace EA::Allocators
