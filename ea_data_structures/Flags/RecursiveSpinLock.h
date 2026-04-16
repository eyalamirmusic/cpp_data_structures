#pragma once

#include "CopyableAtomic.h"
#include <thread>

namespace EA::Locks
{
//A reentrant spinlock: the owning thread can lock() multiple times without
//deadlocking (tracked via std::thread::id plus a hold count). Other threads
//busy-wait until the owning thread has released every held level.
class RecursiveSpinLock
{
    using ID = std::thread::id;

public:
    void lock() noexcept
    {
        auto current = getCurrentID();

        if (id.load() != current)
            while (!tryLock(current)) {}

        ++holders;
    }

    void unlock() noexcept
    {
        --holders;

        if (holders == 0)
            id.store({});
    }

private:
    bool tryLock(ID current) noexcept
    {
        auto null = ID();
        return id.compare_exchange_weak(
            null, current, std::memory_order::release, std::memory_order::relaxed);
    }

    static ID getCurrentID() noexcept { return std::this_thread::get_id(); }

    Atomic<ID> id {};
    int holders = 0;
};

} // namespace EA::Locks