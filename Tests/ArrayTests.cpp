#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>

using namespace nano;

auto arraySizeIsInt = test("Array.size_returns_int") = []
{
    auto a = EA::Array<int, 4>();
    static_assert(std::is_same_v<decltype(a.size()), int>);
    check(a.size() == 4);
};

auto arrayContains = test("Array.contains") = []
{
    auto a = EA::Array<int, 3>();
    a[0] = 10;
    a[1] = 20;
    a[2] = 30;
    check(a.contains(20));
    check(!a.contains(5));
};

auto arrayFill = test("Array.fill") = []
{
    auto a = EA::Array<int, 5>();
    a.fill(7);
    for (auto i = 0; i < a.size(); ++i)
        check(a[i] == 7);
};

auto arraySort = test("Array.sort") = []
{
    auto a = EA::Array<int, 4>();
    a[0] = 4;
    a[1] = 2;
    a[2] = 3;
    a[3] = 1;
    a.sort();
    check(a[0] == 1);
    check(a[1] == 2);
    check(a[2] == 3);
    check(a[3] == 4);
};

auto arrayIndexOf = test("Array.getIndexOf") = []
{
    auto a = EA::Array<int, 3>();
    a[0] = 100;
    a[1] = 200;
    a[2] = 300;
    check(a.getIndexOf(200) == 1);
    check(a.getIndexOf(999) == -1);
};

auto arrayInitializerList = test("Array.initializer_list_construction") = []
{
    auto a = EA::Array<int, 4> {1, 2, 3, 4};
    check(a.size() == 4);
    check(a[0] == 1);
    check(a[1] == 2);
    check(a[2] == 3);
    check(a[3] == 4);
};

auto arrayInitializerListPartial =
    test("Array.initializer_list_partial_fills_rest") = []
{
    auto a = EA::Array<int, 5> {1, 2, 3};
    check(a[0] == 1);
    check(a[1] == 2);
    check(a[2] == 3);
    check(a[3] == 0);
    check(a[4] == 0);
};
