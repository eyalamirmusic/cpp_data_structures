#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Flags/Locks.h>

using namespace nano;

auto spinLockUnlocked = test("PrimitiveSpinLock.starts_unlocked") = []
{
    auto lock = EA::Locks::PrimitiveSpinLock();
    check(!lock.isLocked());
};

auto spinLockLockUnlock = test("PrimitiveSpinLock.lock_and_unlock") = []
{
    auto lock = EA::Locks::PrimitiveSpinLock();
    lock.lock();
    check(lock.isLocked());
    lock.unlock();
    check(!lock.isLocked());
};

auto spinLockTryLock = test("PrimitiveSpinLock.tryLock") = []
{
    auto lock = EA::Locks::PrimitiveSpinLock();
    check(lock.tryLock());
    check(lock.isLocked());
    check(!lock.tryLock());
    lock.unlock();
    check(lock.tryLock());
    lock.unlock();
};

auto scopedSpinLockRAII = test("ScopedSpinLock.RAII_releases_on_scope_exit") = []
{
    auto lock = EA::Locks::PrimitiveSpinLock();
    {
        auto guard = EA::Locks::ScopedSpinLock {lock};
        check(lock.isLocked());
    }
    check(!lock.isLocked());
};
