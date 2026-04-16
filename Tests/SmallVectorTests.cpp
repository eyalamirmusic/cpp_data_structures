#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>

using namespace nano;

auto smallVecDefault = test("SmallVector.defaults_empty_and_static") = []
{
    auto v = EA::SmallVector<int, 4>();
    check(v.empty());
    check(v.isStatic());
};

auto smallVecInitializerList = test("SmallVector.initializer_list") = []
{
    auto v = EA::SmallVector<int, 8> {1, 2, 3};
    check(v.size() == 3);
    check(v[0] == 1);
    check(v[2] == 3);
    check(v.isStatic());
};

auto smallVecAddStaysStatic = test("SmallVector.stays_static_within_capacity") = []
{
    auto v = EA::SmallVector<int, 4>();
    v.add(1);
    v.add(2);
    v.add(3);
    check(v.size() == 3);
    check(v.isStatic());
};

auto smallVecSwitchesToDynamic =
    test("SmallVector.switches_to_dynamic_when_exceeding_capacity") = []
{
    auto v = EA::SmallVector<int, 2>();
    v.add(1);
    v.add(2);
    check(v.isStatic());
    v.add(3);
    check(!v.isStatic());
    check(v.size() == 3);
    check(v[0] == 1);
    check(v[2] == 3);
};

auto smallVecContains = test("SmallVector.contains") = []
{
    auto v = EA::SmallVector<int, 4> {10, 20, 30};
    check(v.contains(20));
    check(!v.contains(99));
};

auto smallVecClear = test("SmallVector.clear") = []
{
    auto v = EA::SmallVector<int, 4> {1, 2, 3};
    v.clear();
    check(v.empty());
};

auto smallVecEraseIf = test("SmallVector.eraseIf") = []
{
    auto v = EA::SmallVector<int, 8> {1, 2, 3, 4, 5};
    v.eraseIf([](int x) { return x % 2 == 0; });
    check(v.size() == 3);
    check(v[0] == 1);
    check(v[1] == 3);
    check(v[2] == 5);
};

auto smallVecSort = test("SmallVector.sort") = []
{
    auto v = EA::SmallVector<int, 8> {3, 1, 2};
    v.sort();
    check(v[0] == 1);
    check(v[1] == 2);
    check(v[2] == 3);
};

auto smallVecPopBack = test("SmallVector.pop_back") = []
{
    auto v = EA::SmallVector<int, 4> {1, 2, 3};
    v.pop_back();
    check(v.size() == 2);
};
