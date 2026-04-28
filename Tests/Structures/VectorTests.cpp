#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/Vector.h>
#include <type_traits>

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

auto vectorReverse = test("Vector.reverse") = []
{
    auto v = EA::Vector<int> {1, 2, 3};
    v.reverse();
    check(v[0] == 3);
    check(v[1] == 2);
    check(v[2] == 1);
};

auto vectorBackFront = test("Vector.back_and_front") = []
{
    auto v = EA::Vector<int> {10, 20, 30};
    check(v.front() == 10);
    check(v.back() == 30);
};

auto vectorPopBack = test("Vector.pop_back") = []
{
    auto v = EA::Vector<int> {1, 2, 3};
    v.pop_back();
    check(v.size() == 2);
    check(v.back() == 2);
};

auto vectorRemoveAt = test("Vector.removeAt") = []
{
    auto v = EA::Vector<int> {10, 20, 30, 40};
    v.removeAt(1);
    check(v.size() == 3);
    check(v[0] == 10);
    check(v[1] == 30);
    check(v[2] == 40);
};

auto vectorInsertAt = test("Vector.insertAt") = []
{
    auto v = EA::Vector<int> {1, 3, 4};
    v.insertAt(1, 2);
    check(v.size() == 4);
    check(v[0] == 1);
    check(v[1] == 2);
    check(v[2] == 3);
};

auto vectorRemoveAllMatches = test("Vector.removeAllMatches") = []
{
    auto v = EA::Vector<int> {1, 2, 1, 3, 1};
    auto removed = v.removeAllMatches(1);
    check(removed == 3);
    check(v.size() == 2);
};

auto vectorLastElementIndex = test("Vector.getLastElementIndex") = []
{
    auto v = EA::Vector<int> {1, 2, 3};
    check(v.getLastElementIndex() == 2);
    v.clear();
    check(v.getLastElementIndex() == -1);
    check(v.getLastValidElementIndex() == 0);
};

auto vectorFilterInPlace = test("Vector.filterInPlace_removes_matches") = []
{
    auto v = EA::Vector<int> {1, 2, 3, 4, 5};
    v.filterInPlace([](int x) { return x % 2 == 0; });
    check(v.size() == 3);
    check(v[0] == 1);
    check(v[1] == 3);
    check(v[2] == 5);
};

auto vectorClear = test("Vector.clear") = []
{
    auto v = EA::Vector<int> {1, 2, 3};
    v.clear();
    check(v.empty());
    check(v.size() == 0);
};

auto vectorEmplaceBack = test("Vector.emplace_back") = []
{
    auto v = EA::Vector<int>();
    auto& ref = v.emplace_back(42);
    check(v.size() == 1);
    check(ref == 42);
    check(&v[0] == &ref);
};

auto vectorEqualityEqual = test("Vector.operator==_equal") = []
{
    auto a = EA::Vector<int> {1, 2, 3};
    auto b = EA::Vector<int> {1, 2, 3};
    check(a == b);
    check(!(a != b));
};

auto vectorEqualityDifferentValues =
    test("Vector.operator==_different_values") = []
{
    auto a = EA::Vector<int> {1, 2, 3};
    auto b = EA::Vector<int> {1, 2, 4};
    check(!(a == b));
    check(a != b);
};

auto vectorEqualityDifferentSizes =
    test("Vector.operator==_different_sizes") = []
{
    auto a = EA::Vector<int> {1, 2, 3};
    auto b = EA::Vector<int> {1, 2};
    check(!(a == b));
    check(a != b);
};

auto vectorEqualityEmpty = test("Vector.operator==_empty") = []
{
    auto a = EA::Vector<int>();
    auto b = EA::Vector<int>();
    check(a == b);
    check(!(a != b));
};

auto vectorEqualityConst = test("Vector.operator==_works_on_const") = []
{
    const auto a = EA::Vector<int> {1, 2, 3};
    const auto b = EA::Vector<int> {1, 2, 3};
    check(a == b);
    check(!(a != b));
};
