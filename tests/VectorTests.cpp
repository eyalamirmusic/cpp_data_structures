#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>

using namespace nano;

auto vectorSizeIsInt = test("Vector.size_returns_int") = []
{
    auto v = EA::Vector<int> {1, 2};
    static_assert(std::is_same_v<decltype(v.size()), int>);
    check(v.size() == 2);
};

auto vectorAddAndIndex = test("Vector.add_and_index") = []
{
    auto v = EA::Vector<int>();
    v.add(1);
    v.add(2);
    v.add(3);
    check(v.size() == 3);
    check(v[0] == 1);
    check(v[2] == 3);
};

auto vectorContains = test("Vector.contains") = []
{
    auto v = EA::Vector<int> {10, 20, 30};
    check(v.contains(20));
    check(!v.contains(99));
};

auto vectorAddIfNotThere = test("Vector.addIfNotThere") = []
{
    auto v = EA::Vector<int> {1, 2, 3};
    check(v.addIfNotThere(4));
    check(!v.addIfNotThere(2));
    check(v.size() == 4);
};

auto vectorEraseIf = test("Vector.eraseIf") = []
{
    auto v = EA::Vector<int> {1, 2, 3, 4, 5};
    v.eraseIf([](int x) { return x % 2 == 0; });
    check(v.size() == 3);
    check(v[0] == 1);
    check(v[1] == 3);
    check(v[2] == 5);
};

auto vectorSort = test("Vector.sort_ascending_and_descending") = []
{
    auto v = EA::Vector<int> {3, 1, 4, 1, 5, 9, 2, 6};
    v.sort();
    check(v[0] == 1);
    check(v[v.size() - 1] == 9);

    v.sort(false);
    check(v[0] == 9);
    check(v[v.size() - 1] == 1);
};

auto vectorFind = test("Vector.find_returns_pointer_or_null") = []
{
    auto v = EA::Vector<int> {5, 10, 15};
    auto* hit = v.find(10);
    check(hit != nullptr);
    check(*hit == 10);
    check(v.find(42) == nullptr);
};
