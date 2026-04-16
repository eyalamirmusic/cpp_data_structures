#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Flags/Bool.h>

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
