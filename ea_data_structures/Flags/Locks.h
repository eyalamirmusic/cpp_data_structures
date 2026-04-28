#pragma once

#include "CopyableAtomic.h"
#include "Bool.h"
#include "SpinHint.h"

//A primitive version of a spinlocks for simple tasks when the lock
//Is really held for a very short period of time

namespace EA::Locks
{
//A boolean test-and-set flag backed by std::atomic<bool>. Used as the
//primitive behind PrimitiveSpinLock; distinct from EA::AtomicFlag in
//CopyableAtomic.h, which is a monotonic update counter.
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
        while (true)
        {
            if (tryLock())
                return;

            //Read-only spin until the lock looks free, so contending
            //cores can keep the cache line in Shared state instead of
            //ping-ponging it via repeated test_and_set writes.
            while (locked.test())
                spinHint();
        }
    }

    bool tryLock() noexcept { return !locked.test_and_set(); }

    bool isLocked() const noexcept { return locked.test(); }
    void unlock() noexcept { locked.clear(); }

private:
    AtomicFlag locked;
};

//RAII guard for any lock with lock()/unlock() members (e.g. PrimitiveSpinLock
//or RecursiveSpinLock). Acquires on construction, releases on destruction.
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
