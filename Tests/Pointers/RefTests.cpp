#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Pointers/OwningPointer.h>
#include <ea_data_structures/Pointers/Ref.h>

using namespace nano;

auto refFromReference = test("Ref.constructs_from_reference") = []
{
    auto x = 42;
    auto r = EA::Ref<int>(x);
    check(r.get() == &x);
    check(*r == 42);
};

auto refFromPointer = test("Ref.constructs_from_pointer") = []
{
    auto x = 7;
    auto r = EA::Ref<int>(&x);
    check(*r == 7);
};

auto refFromOwningPointer = test("Ref.constructs_from_OwningPointer") = []
{
    auto p = EA::makeOwned<int>(5);
    auto r = EA::Ref<int>(p);
    check(*r == 5);
};
