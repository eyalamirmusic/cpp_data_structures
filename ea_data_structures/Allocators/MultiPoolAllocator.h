#pragma once

#include <algorithm>
#include "../Structures/Vector.h"
#include <iostream>

namespace EA::Allocators
{
class DynamicMemory
{
public:
    DynamicMemory(size_t size) { data.reserve(size); }

    void* allocate(size_t size)
    {
        data.resize(size);
        return data.data();
    }

    bool operator==(const void* other) const { return getAddr() == other; }

private:
    const void* getAddr() const { return data.data(); }

    std::vector<std::byte> data;
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
        return nullptr;
    }

    bool empty() const { return resources.empty(); }
    void create(size_t size) { push_back(std::make_unique<DynamicMemory>(size)); }

    std::vector<SingleResource> resources;
};

class MemoryPool
{
public:
    MemoryPool(int poolObjectSizeToUse,
               int freeListInitialSize = 1000,
               int numReservedObjects = 10000)
        : poolObjectSize((size_t) poolObjectSizeToUse)
        , free((size_t) numReservedObjects)
        , used((size_t) numReservedObjects)
    {
        for (int index = 0; index < freeListInitialSize; ++index)
            free.create(poolObjectSize);
    }

    void* allocate(size_t size)
    {
        if (free.empty())
            free.create(std::max(size, poolObjectSize));

        return allocateFromFreePool(size);
    }

    void deallocate(const void* data)
    {
        if (auto u = used.findAndPop(data))
            free.push_back(std::move(u));
    }

private:
    void* allocateFromFreePool(size_t size)
    {
        auto last = free.pop_back();
        auto addr = last->allocate(size);
        used.push_back(std::move(last));

        return addr;
    }

    void addFreeObject() { free.create(poolObjectSize); }

    size_t poolObjectSize = 0;
    ResourceContainer free;
    ResourceContainer used;
};

class MemoryPoolResource final : public std::pmr::memory_resource
{
public:
    MemoryPoolResource(int objectSize = 1024 * 1024,
                       int freeListInitial = 1000,
                       int reservedObjects = 10000)
        : pool(objectSize, freeListInitial, reservedObjects)
    {
    }

private:
    void* do_allocate(size_t bytes, size_t) override { return pool.allocate(bytes); }
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
} // namespace EA::Allocators
