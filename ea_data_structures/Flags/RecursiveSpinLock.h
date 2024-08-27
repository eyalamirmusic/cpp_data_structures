#pragma once

#include "CopyableAtomic.h"
#include <thread>

namespace EA::Locks
{
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

    void unlock()
    {
        --holders;

        if (holders == 0)
            id.store({});
    }

    bool tryLock() noexcept { return tryLock(getCurrentID()); }

    bool isLocked() const noexcept { return id.load() != ID(); }

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