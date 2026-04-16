#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Flags/RecursiveSpinLock.h>
#include <thread>

using namespace nano;

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
