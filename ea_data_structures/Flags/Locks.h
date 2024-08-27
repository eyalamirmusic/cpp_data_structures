#pragma once

#include "CopyableAtomic.h"
#include "Bool.h"

//A primitive version of a spinlocks for simple tasks when the lock
//Is really held for a very short period of time

namespace EA::Locks
{
class AtomicFlag
{
public:
    bool test_and_set() noexcept { return atomic.exchange(true); }
    bool test() const noexcept { return atomic.load(); }
    void clear() noexcept { atomic.store(false); }

private:
    Atomic<bool> atomic {false};
};

class PrimitiveSpinLock
{
public:
    void lock() noexcept
    {
        while (!tryLock()) {}
    }

    bool tryLock() noexcept { return !locked.test_and_set(); }

    bool isLocked() const noexcept { return locked.test(); }
    void unlock() noexcept { locked.clear(); }

private:
    AtomicFlag locked;
};

template<typename LockType>
class ScopedSpinLock
{
public:
    explicit ScopedSpinLock(LockType& lockToUse)
        : lock(lockToUse)
    {
        lock.lock();
    }

    ~ScopedSpinLock() { lock.unlock(); }

private:
    LockType& lock;
};

} // namespace EA::Locks
