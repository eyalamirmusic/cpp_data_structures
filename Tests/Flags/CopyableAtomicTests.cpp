#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Flags/CopyableAtomic.h>

using namespace nano;

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
