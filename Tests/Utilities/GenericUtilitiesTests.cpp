#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Utilities/GenericUtilities.h>

using namespace nano;

auto equalsToAnyMatches = test("equalsToAny.returns_true_on_match") = []
{
    check(EA::equalsToAny(3, 1, 2, 3, 4));
    check(EA::equalsToAny(1, 1));
};

auto equalsToAnyNoMatch = test("equalsToAny.returns_false_when_absent") = []
{
    check(!EA::equalsToAny(5, 1, 2, 3, 4));
};

auto equalsToAllUniform = test("equalsToAll.returns_true_when_all_match") = []
{
    check(EA::equalsToAll(3, 3, 3, 3));
};

auto equalsToAllMixed = test("equalsToAll.returns_false_when_any_differs") = []
{
    check(!EA::equalsToAll(3, 3, 4, 3));
};

auto toggleBoolFlip = test("toggleBool.flips_value") = []
{
    auto b = false;
    EA::toggleBool(b);
    check(b);
    EA::toggleBool(b);
    check(!b);
};
