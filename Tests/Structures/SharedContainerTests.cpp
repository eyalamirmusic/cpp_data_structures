//Shared behavior tests for the vector-like containers: every case is written
//once against a generic container type and registered for Vector, SmallVector
//and StaticVector, so the helpers they share can't silently diverge.
#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/Vector.h>
#include <ea_data_structures/Structures/SmallVector.h>
#include <ea_data_structures/Structures/StaticVector.h>
#include <initializer_list>

using namespace nano;

namespace
{
template <typename C>
bool equals(const C& container, std::initializer_list<int> expected)
{
    if ((int) container.size() != (int) expected.size())
        return false;

    int index = 0;

    for (auto value: expected)
    {
        if (container[index] != value)
            return false;

        ++index;
    }

    return true;
}
} // namespace

#define SHARED_TEST(testName, caseFunc) \
    auto reg_##caseFunc##_vector = nano::test("Shared." testName ".Vector") = \
        [] { caseFunc<EA::Vector<int>>(); }; \
    auto reg_##caseFunc##_small = \
        nano::test("Shared." testName ".SmallVector") = \
            [] { caseFunc<EA::SmallVector<int, 8>>(); }; \
    auto reg_##caseFunc##_static = \
        nano::test("Shared." testName ".StaticVector") = \
            [] { caseFunc<EA::StaticVector<int, 16>>(); };

template <typename C>
void fillCase()
{
    auto v = C {0, 0, 0, 0};
    v.fill(7);
    check(equals(v, {7, 7, 7, 7}));

    v.fill(9, 2);
    check(equals(v, {9, 9, 7, 7}));
}
SHARED_TEST("fill", fillCase)

template <typename C>
void mixFromCase()
{
    auto v = C {1, 2, 3};
    auto other = C {10, 20, 30};
    v.mixFrom(other);
    check(equals(v, {11, 22, 33}));
}
SHARED_TEST("mixFrom", mixFromCase)

template <typename C>
void addFromCase()
{
    auto v = C {1, 2};
    auto other = C {3, 4};
    v.addFrom(other);
    check(equals(v, {1, 2, 3, 4}));
}
SHARED_TEST("addFrom", addFromCase)

template <typename C>
void addFromIndexesCase()
{
    auto v = C {};
    auto other = C {5, 6, 7};
    v.addFromIndexes(other, {0, 2});
    check(equals(v, {5, 7}));
}
SHARED_TEST("addFromIndexes", addFromIndexesCase)

template <typename C>
void fillFromCase()
{
    auto v = C {9, 9, 9, 9, 9};
    auto other = C {1, 2, 3};
    v.fillFrom(other);
    check(equals(v, {1, 2, 3}));
}
SHARED_TEST("fillFrom", fillFromCase)

template <typename C>
void copyFromRangeCase()
{
    auto src = C {1, 2, 3, 4, 5};
    auto dst = C {};

    dst.copyFrom(src, 1, 3);
    check(equals(dst, {2, 3, 4}));

    //Requesting more items than available clamps to the source size
    dst.copyFrom(src, 3, 10);
    check(equals(dst, {4, 5}));
}
SHARED_TEST("copyFrom_range", copyFromRangeCase)

template <typename C>
void eraseIfCase()
{
    auto v = C {1, 2, 3, 4, 5, 6};
    check(v.eraseIf([](int x) { return x % 2 == 0; }));
    check(equals(v, {1, 3, 5}));
    check(!v.eraseIf([](int x) { return x > 100; }));
}
SHARED_TEST("eraseIf", eraseIfCase)

template <typename C>
void removeRangeCase()
{
    auto v = C {1, 2, 3, 4, 5};
    v.removeRange(1, 3);
    check(equals(v, {1, 4, 5}));

    auto tail = C {1, 2, 3, 4, 5};
    tail.removeRange(2, 5);
    check(equals(tail, {1, 2}));
}
SHARED_TEST("removeRange", removeRangeCase)

template <typename C>
void popBackCase()
{
    auto v = C {1, 2};
    v.pop_back();
    check(equals(v, {1}));

    v.pop_back();
    check(v.empty());

    v.pop_back(); //no-op on empty
    check(v.empty());
}
SHARED_TEST("pop_back", popBackCase)

template <typename C>
void sortCase()
{
    auto v = C {3, 1, 2};
    v.sort();
    check(equals(v, {1, 2, 3}));

    v.sort(false);
    check(equals(v, {3, 2, 1}));
}
SHARED_TEST("sort", sortCase)

template <typename C>
void sortPredicateCase()
{
    auto v = C {1, 3, 2};
    v.sort([](int a, int b) { return a > b; });
    check(equals(v, {3, 2, 1}));

    v.sort([](int a, int b) { return a > b; }, false);
    check(equals(v, {1, 2, 3}));
}
SHARED_TEST("sort_predicate", sortPredicateCase)

template <typename C>
void reverseCase()
{
    auto v = C {1, 2, 3};
    v.reverse();
    check(equals(v, {3, 2, 1}));
}
SHARED_TEST("reverse", reverseCase)

template <typename C>
void findCase()
{
    auto v = C {10, 20, 30};
    check(v.getIndexOf(20) == 1);
    check(v.getIndexOf(99) == -1);

    auto* found = v.find(20);
    check(found != nullptr && *found == 20);
    check(v.find(99) == nullptr);

    check(v.contains(30));
    check(!v.contains(99));

    check(v.addIfNotThere(40));
    check(!v.addIfNotThere(40));
    check(equals(v, {10, 20, 30, 40}));
}
SHARED_TEST("find", findCase)

template <typename C>
void removeAllMatchesCase()
{
    auto v = C {1, 2, 1, 3, 1};
    check(v.removeAllMatches(1) == 3);
    check(equals(v, {2, 3}));
}
SHARED_TEST("removeAllMatches", removeAllMatchesCase)

template <typename C>
void transformCase()
{
    auto v = C {1, 2, 3};
    auto doubled = v.transform([](int x) { return x * 2; });
    check(equals(doubled, {2, 4, 6}));
}
SHARED_TEST("transform", transformCase)

template <typename C>
void filterCase()
{
    auto v = C {1, 2, 3, 4, 5};
    auto evens = v.filter([](int x) { return x % 2 == 0; });
    check(equals(evens, {2, 4}));

    v.filterInPlace([](int x) { return x % 2 == 0; });
    check(equals(v, {1, 3, 5}));
}
SHARED_TEST("filter", filterCase)

template <typename C>
void copyFilteredToCase()
{
    auto v = C {1, 2, 3, 4};
    auto target = C {0, 0};
    v.copyFilteredTo(target, [](int x) { return x > 2; });
    check(equals(target, {3, 4}));
}
SHARED_TEST("copyFilteredTo", copyFilteredToCase)

template <typename C>
void addFilteredToCase()
{
    auto v = C {1, 2, 3, 4};
    auto target = C {};
    v.addFilteredTo(target, [](int x) { return x > 2; });
    check(equals(target, {3, 4}));
}
SHARED_TEST("addFilteredTo", addFilteredToCase)

template <typename C>
void relativeCase()
{
    auto v = C {10, 20, 30, 40};

    check(v.template getIndexAsRelative<float>(-1) == -1.0f);
    check(v.template getIndexAsRelative<float>(0) == 0.0f);
    check(v.template getIndexAsRelative<float>(3) == 1.0f);

    check(v.getRelativeIndex(0.0f) == 0);
    check(v.getRelativeIndex(0.5f) == 2);
    check(v.getRelativeIndex(1.0f) == 3);

    check(v.getRelative(0.0f) == 10);
    check(v.getRelative(1.0f) == 40);

    const auto& constRef = v;
    check(constRef.getRelative(0.0f) == 10);

    check(v.template getRelativeIndexOf<float>(40) == 1.0f);
}
SHARED_TEST("relative_indexing", relativeCase)

template <typename C>
void insertCase()
{
    auto v = C {1, 3};
    v.insert(1, 2);
    check(equals(v, {1, 2, 3}));

    v.insert(0, 0);
    check(equals(v, {0, 1, 2, 3}));

    v.insert(4, 4);
    check(equals(v, {0, 1, 2, 3, 4}));
}
SHARED_TEST("insert", insertCase)
