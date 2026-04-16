#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>
#include <thread>

using namespace nano;

auto boolDefaultsFalse = test("Bool.defaults_to_false") = []
{
    auto b = EA::Bool();
    check(!b);
    check(b == false);
};

auto boolConstructsFromValue = test("Bool.constructs_from_bool") = []
{
    auto b = EA::Bool(true);
    check(b);
    check(b == true);
};

auto boolAssigns = test("Bool.assigns_from_bool") = []
{
    auto b = EA::Bool();
    b = true;
    check(b);
    b = false;
    check(!b);
};

auto boolEqualityInequality = test("Bool.equality_and_inequality") = []
{
    auto a = EA::Bool(true);
    auto b = EA::Bool(true);
    auto c = EA::Bool(false);
    check(a == b);
    check(a != c);
    check(!(a == c));
};

auto atomicStoreLoad = test("Atomic.store_and_load") = []
{
    auto a = EA::Atomic<int>(0);
    a.store(42);
    check(a.load() == 42);
};

auto atomicInitialValue = test("Atomic.initial_value") = []
{
    auto a = EA::Atomic<int>(7);
    check(a.load() == 7);
};

auto atomicIncrement = test("Atomic.increment") = []
{
    auto a = EA::Atomic<int>(0);
    ++a;
    ++a;
    check(a.load() == 2);
};

auto copyableAtomicCopyable = test("CopyableAtomic.is_copy_constructible") = []
{
    auto a = EA::CopyableAtomic<int>(5);
    auto b = a;
    check(b.load() == 5);
};

auto copyableAtomicAssign = test("CopyableAtomic.copy_assignment") = []
{
    auto a = EA::CopyableAtomic<int>(3);
    auto b = EA::CopyableAtomic<int>(0);
    b = a;
    check(b.load() == 3);
};

auto copyableAtomicImplicitConversion =
    test("CopyableAtomic.converts_to_underlying") = []
{
    auto a = EA::CopyableAtomic<int>(10);
    int value = a;
    check(value == 10);
};

auto atomicFlagStartsZero = test("AtomicFlag.starts_at_zero") = []
{
    auto flag = EA::AtomicFlag();
    check(flag.load() == 0);
};

auto atomicFlagUpdateIncrements = test("AtomicFlag.update_increments_counter") = []
{
    auto flag = EA::AtomicFlag();
    flag.update();
    flag.update();
    flag.update();
    check(flag.load() == 3);
};

auto atomicWrapperStoreLoad = test("AtomicWrapper.store_and_load") = []
{
    auto w = EA::AtomicWrapper<int>(10);
    check(w.load() == 10);
    w.store(7);
    check(w.load() == 7);
};

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

auto recursiveSpinLockReentrant =
    test("RecursiveSpinLock.same_thread_can_lock_recursively") = []
{
    auto lock = EA::Locks::RecursiveSpinLock();
    lock.lock();
    lock.lock();
    lock.lock();
    lock.unlock();
    lock.unlock();
    lock.unlock();
    check(true);
};

auto recursiveSpinLockAnotherThread =
    test("RecursiveSpinLock.releases_for_other_threads") = []
{
    auto lock = EA::Locks::RecursiveSpinLock();
    lock.lock();
    lock.unlock();

    auto acquired = false;
    auto t = std::thread(
        [&]
        {
            lock.lock();
            acquired = true;
            lock.unlock();
        });
    t.join();
    check(acquired);
};
