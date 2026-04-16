#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>

using namespace nano;

auto staticVecDefault = test("StaticVector.defaults_empty") = []
{
    auto v = EA::StaticVector<int, 4>();
    check(v.empty());
    check(v.size() == 0);
};

auto staticVecInitializerList = test("StaticVector.initializer_list") = []
{
    auto v = EA::StaticVector<int, 8> {1, 2, 3};
    check(v.size() == 3);
    check(v[0] == 1);
    check(v[2] == 3);
};

auto staticVecAdd = test("StaticVector.add_within_capacity") = []
{
    auto v = EA::StaticVector<int, 4>();
    v.add(10);
    v.add(20);
    check(v.size() == 2);
    check(v[0] == 10);
    check(v[1] == 20);
};

auto staticVecAddBeyondCapacityIgnored =
    test("StaticVector.add_beyond_capacity_is_ignored") = []
{
    auto v = EA::StaticVector<int, 2>();
    v.add(1);
    v.add(2);
    v.add(3);
    check(v.size() == 2);
    check(v[0] == 1);
    check(v[1] == 2);
};

auto staticVecClear = test("StaticVector.clear") = []
{
    auto v = EA::StaticVector<int, 4> {1, 2, 3};
    v.clear();
    check(v.empty());
    check(v.size() == 0);
};

auto staticVecContains = test("StaticVector.contains") = []
{
    auto v = EA::StaticVector<int, 4> {5, 10, 15};
    check(v.contains(10));
    check(!v.contains(999));
};

auto staticVecAddIfNotThere = test("StaticVector.addIfNotThere") = []
{
    auto v = EA::StaticVector<int, 4> {1, 2};
    check(v.addIfNotThere(3));
    check(!v.addIfNotThere(2));
    check(v.size() == 3);
};

auto staticVecEraseIf = test("StaticVector.eraseIf") = []
{
    auto v = EA::StaticVector<int, 8> {1, 2, 3, 4, 5};
    v.eraseIf([](int x) { return x % 2 == 0; });
    check(v.size() == 3);
    check(v[0] == 1);
    check(v[1] == 3);
    check(v[2] == 5);
};

auto staticVecSort = test("StaticVector.sort") = []
{
    auto v = EA::StaticVector<int, 8> {3, 1, 4, 1, 5};
    v.sort();
    check(v[0] == 1);
    check(v[v.getLastElementIndex()] == 5);
};

auto staticVecCopy = test("StaticVector.copy_constructor") = []
{
    auto a = EA::StaticVector<int, 4> {1, 2, 3};
    auto b = a;
    check(b.size() == 3);
    check(b[0] == 1);
    check(b[2] == 3);
};

auto staticVecPopBack = test("StaticVector.pop_back") = []
{
    auto v = EA::StaticVector<int, 4> {1, 2, 3};
    v.pop_back();
    check(v.size() == 2);
    check(v[v.getLastElementIndex()] == 2);
};

auto staticVecRemoveAt = test("StaticVector.removeAt_shifts_remaining") = []
{
    auto v = EA::StaticVector<int, 4> {10, 20, 30};
    v.removeAt(0);
    check(v.size() == 2);
    check(v[0] == 20);
    check(v[1] == 30);
};

auto staticVecResizeGrows = test("StaticVector.resize_grows") = []
{
    auto v = EA::StaticVector<int, 8>();
    v.resize(3);
    check(v.size() == 3);
};

auto staticVecFill = test("StaticVector.fill") = []
{
    auto v = EA::StaticVector<int, 4>();
    v.resize(4);
    v.fill(7);
    for (auto i = 0; i < v.size(); ++i)
        check(v[i] == 7);
};

auto staticVecGetIndexOf = test("StaticVector.getIndexOf") = []
{
    auto v = EA::StaticVector<int, 4> {100, 200, 300};
    check(v.getIndexOf(200) == 1);
    check(v.getIndexOf(99) == -1);
};
