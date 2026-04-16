#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/CopyOnWrite.h>

using namespace nano;

auto cowConstructs = test("CopyOnWrite.constructs_value") = []
{
    auto c = EA::CopyOnWrite<int>(42);
    check(*c == 42);
};

auto cowCopyShares = test("CopyOnWrite.copy_shares_pointer") = []
{
    auto a = EA::CopyOnWrite<int>(5);
    auto b = a;
    check(a.get() == b.get());
};

auto cowAssignDiverges =
    test("CopyOnWrite.assign_from_value_replaces_storage") = []
{
    auto a = EA::CopyOnWrite<int>(1);
    auto b = a;
    check(a.get() == b.get());
    b = 99;
    check(a.get() != b.get());
    check(*a == 1);
    check(*b == 99);
};

auto cowPointerAccess = test("CopyOnWrite.arrow_operator") = []
{
    struct Thing
    {
        int value = 0;
    };
    auto c = EA::CopyOnWrite<Thing>(Thing {7});
    check(c->value == 7);
};
