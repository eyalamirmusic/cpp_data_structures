#include <NanoTest/NanoTest.h>
#include <algorithm>
#include <array>
#include <memory>
#include <ea_data_structures/Structures/StaticVector.h>
#include <ea_data_structures/Structures/Vector.h>
#include <ea_data_structures/Utilities/VectorUtilities.h>
#include <vector>

using namespace nano;

auto rangesLinearMap = test("Ranges.linearMap_midpoint") = []
{
    auto mapped = EA::Ranges::linearMap(5.0f, 0.0f, 10.0f, 0.0f, 100.0f);
    check(mapped == 50.0f);
};

auto rangesMapNormalized = test("Ranges.map_from_unit_interval") = []
{
    auto mapped = EA::Ranges::map(0.5f, 0.0f, 10.0f);
    check(mapped == 5.0f);
};

auto rangesMap5Arg = test("Ranges.map_with_full_ranges") = []
{
    auto mapped = EA::Ranges::map(2.0f, 0.0f, 4.0f, 100.0f, 200.0f);
    check(mapped == 150.0f);
};

auto rangesGetIndexProportion = test("Ranges.getIndexProprtion_clamps_at_end") = []
{
    check(EA::Ranges::getIndexProprtion(0.0f, 4) == 0);
    check(EA::Ranges::getIndexProprtion(1.0f, 4) == 3);
    check(EA::Ranges::getIndexProprtion(0.5f, 4) == 2);
};

auto vectorsContainsStandalone = test("Vectors.contains_on_std_vector") = []
{
    auto v = std::vector<int> {1, 2, 3};
    check(EA::Vectors::contains(v, 2));
    check(!EA::Vectors::contains(v, 99));
};

auto vectorsGetIndexOfStandalone = test("Vectors.getIndexOf_on_std_vector") = []
{
    auto v = std::vector<int> {10, 20, 30};
    check(EA::Vectors::getIndexOf(v, 20) == 1);
    check(EA::Vectors::getIndexOf(v, 99) == -1);
};

auto vectorsGetIndexIfStandalone = test("Vectors.getIndexIf_finds_first_match") = []
{
    auto v = std::vector<int> {1, 2, 3, 4};
    auto idx = EA::Vectors::getIndexIf(v, [](int x) { return x > 2; });
    check(idx == 2);
};

auto vectorsGetIndexOfComparison = test("Vectors.getIndexOfComparison") = []
{
    auto v = std::vector<int> {5, 10, 15, 20};
    auto idx =
        EA::Vectors::getIndexOfComparison(v, [](const auto& e) { return e == 15; });
    check(idx == 2);
    auto missing =
        EA::Vectors::getIndexOfComparison(v, [](const auto& e) { return e == 999; });
    check(missing == -1);
};

auto vectorsGetIndexOfReverse = test("Vectors.getIndexOfReverse") = []
{
    auto v = std::vector<int> {1, 2, 3};
    check(EA::Vectors::getIndexOfReverse(v, 2) == 1);
    check(EA::Vectors::getIndexOfReverse(v, 99) == -1);
};

auto vectorsStableSort = test("Vectors.stableSort_ascending_and_descending") = []
{
    auto v = std::vector<int> {3, 1, 4, 1, 5, 9, 2, 6};
    EA::Vectors::stableSort(v);
    check(v.front() == 1);
    check(v.back() == 9);

    EA::Vectors::stableSort(v, false);
    check(v.front() == 9);
    check(v.back() == 1);
};

auto vectorsStableSortComparator = test("Vectors.stableSort_with_comparator") = []
{
    auto v = std::vector<int> {3, 1, 2};
    EA::Vectors::stableSort(v, [](int a, int b) { return a > b; });
    check(v[0] == 3);
    check(v[1] == 2);
    check(v[2] == 1);
};

auto vectorsSortComparator = test("Vectors.sort_with_comparator") = []
{
    auto v = std::vector<int> {3, 1, 2};
    EA::Vectors::sort(v, [](int a, int b) { return a > b; });
    check(v[0] == 3);
    check(v[1] == 2);
    check(v[2] == 1);
};

auto vectorsCopyInto = test("Vectors.copyInto_resizes_and_copies") = []
{
    auto src = EA::Vector<int> {1, 2, 3};
    auto dst = EA::Vector<int> {9, 9};
    EA::Vectors::copyInto(src, dst);
    check(dst.size() == 3);
    check(dst[0] == 1);
    check(dst[1] == 2);
    check(dst[2] == 3);
};

auto vectorsTransform = test("Vectors.transform_maps_each_element") = []
{
    auto src = EA::StaticVector<int, 5> {1, 2, 3};
    auto dst = EA::Vectors::transform(src, [](int x) { return x * 2; });
    check(dst.size() == 3);
    check(dst[0] == 2);
    check(dst[1] == 4);
    check(dst[2] == 6);
};

auto vectorsFilter = test("Vectors.filter_keeps_matches") = []
{
    auto v = std::vector<int> {1, 2, 3, 4, 5};
    auto result = EA::Vectors::filter(v, [](int x) { return x % 2 == 0; });
    check(result.size() == 2u);
    check(result[0] == 2);
    check(result[1] == 4);
};

auto vectorsFold = test("Vectors.fold_left_folds_elements") = []
{
    auto v = EA::Vector<int> {1, 2, 3, 4};
    auto sum = EA::Vectors::fold(v, [](int a, int b) { return a + b; });
    check(sum == 10);
};

auto vectorsFoldr = test("Vectors.foldr_right_folds_elements") = []
{
    auto v = EA::Vector<int> {1, 2, 3, 4};
    auto sum = EA::Vectors::foldr(v, [](int a, int b) { return a + b; });
    check(sum == 10);
};

auto vectorsEraseIfStandalone = test("Vectors.eraseIf_on_std_vector") = []
{
    auto v = std::vector<int> {1, 2, 3, 4};
    auto removed = EA::Vectors::eraseIf(v, [](int x) { return x > 2; });
    check(removed);
    check(v.size() == 2u);
    check(v[0] == 1);
    check(v[1] == 2);
};

auto vectorsAddIfNotThereStandalone =
    test("Vectors.addIfNotThere_on_std_vector") = []
{
    auto v = std::vector<int>();
    check(EA::Vectors::addIfNotThere(v, 1));
    check(EA::Vectors::addIfNotThere(v, 2));
    check(!EA::Vectors::addIfNotThere(v, 1));
    check(v.size() == 2u);
};

auto vectorsReverseStandalone = test("Vectors.reverse_on_std_vector") = []
{
    auto v = std::vector<int> {1, 2, 3};
    EA::Vectors::reverse(v);
    check(v[0] == 3);
    check(v[2] == 1);
};

auto vectorsSortStandalone = test("Vectors.sort_forward_and_backward") = []
{
    auto v = std::vector<int> {3, 1, 2};
    EA::Vectors::sort(v);
    check(v[0] == 1);
    EA::Vectors::sort(v, false);
    check(v[0] == 3);
};

auto vectorsZipWith = test("Vectors.zipWith_walks_pairs") = []
{
    auto a = std::vector<int> {1, 2, 3};
    auto b = std::vector<int> {10, 20, 30};
    auto sum = 0;
    EA::Vectors::zipWith(a, b, [&](int x, int y) { sum += x * y; });
    check(sum == 1 * 10 + 2 * 20 + 3 * 30);
};

auto vectorsZipWithIndexed = test("Vectors.zipWithIndexed_passes_index") = []
{
    auto a = std::vector<int> {1, 2, 3};
    auto b = std::vector<int> {10, 20, 30};
    auto lastIndex = -1;
    EA::Vectors::zipWithIndexed(a, b, [&](int, int, int idx) { lastIndex = idx; });
    check(lastIndex == 2);
};

auto vectorsIsStdVector = test("Vectors.isStdVector_detects_std_vector") = []
{
    static_assert(EA::Vectors::isStdVector<std::vector<int>>());
    static_assert(!EA::Vectors::isStdVector<int>());
    check(true);
};

auto vectorsRemoveAt = test("Vectors.removeAt_erases_index") = []
{
    auto v = std::vector<int> {10, 20, 30};
    EA::Vectors::removeAt(v, 1);
    check(v.size() == 2u);
    check(v[0] == 10);
    check(v[1] == 30);
};

auto vectorsRemoveAllMatches = test("Vectors.removeAllMatches_counts") = []
{
    auto v = std::vector<int> {1, 2, 1, 3, 1};
    auto target = 1;
    auto removed = EA::Vectors::removeAllMatches(v, target);
    check(removed == 3);
    check(v.size() == 2u);
};

auto vectorsStableSortIsStable = test("Vectors.stableSort_keeps_ties_in_order") = []
{
    auto v = std::vector<std::pair<int, int>> {};

    for (int index = 0; index < 100; ++index)
        v.push_back({index % 5, index});

    EA::Vectors::stableSort(
        v, [](const auto& a, const auto& b) { return a.first < b.first; });

    for (size_t index = 1; index < v.size(); ++index)
    {
        check(v[index - 1].first <= v[index].first);

        if (v[index - 1].first == v[index].first)
            check(v[index - 1].second < v[index].second);
    }
};

auto vectorsStableSortMatchesStd =
    test("Vectors.stableSort_matches_std_on_large_input") = []
{
    auto v = std::vector<int> {};
    auto seed = 12345u;

    for (int index = 0; index < 5000; ++index)
    {
        seed = seed * 1664525u + 1013904223u;
        v.push_back((int) (seed % 1000u));
    }

    auto expected = v;
    std::sort(expected.begin(), expected.end());

    EA::Vectors::stableSort(v);
    check(v == expected);

    EA::Vectors::stableSort(v, false);
    std::reverse(expected.begin(), expected.end());
    check(v == expected);
};

auto vectorsStableSortMoveOnly = test("Vectors.stableSort_move_only_elements") = []
{
    auto v = std::vector<std::unique_ptr<int>> {};

    for (int index = 50; index > 0; --index)
        v.push_back(std::make_unique<int>(index));

    EA::Vectors::stableSort(v, [](const auto& a, const auto& b) { return *a < *b; });

    for (int index = 0; index < 50; ++index)
        check(*v[(size_t) index] == index + 1);
};

auto vectorsSortMatchesStd =
    test("Vectors.sort_matches_std_sort_on_large_input") = []
{
    auto v = std::vector<int> {};
    auto seed = 98765u;

    for (int index = 0; index < 10000; ++index)
    {
        seed = seed * 1664525u + 1013904223u;
        v.push_back((int) (seed % 500u));
    }

    auto expected = v;
    std::sort(expected.begin(), expected.end());

    EA::Vectors::sort(v);
    check(v == expected);

    EA::Vectors::sort(v, false);
    std::reverse(expected.begin(), expected.end());
    check(v == expected);
};

auto vectorsSortAdversarialPatterns =
    test("Vectors.sort_matches_std_sort_on_adversarial_patterns") = []
{
    auto patterns = std::vector<std::vector<int>> {};
    auto sorted = std::vector<int> {};
    auto reversed = std::vector<int> {};
    auto allEqual = std::vector<int> {};
    auto organPipe = std::vector<int> {};
    auto fewUniques = std::vector<int> {};

    for (int index = 0; index < 3000; ++index)
    {
        sorted.push_back(index);
        reversed.push_back(3000 - index);
        allEqual.push_back(42);
        organPipe.push_back(index < 1500 ? index : 3000 - index);
        fewUniques.push_back(index % 3);
    }

    patterns.push_back(sorted);
    patterns.push_back(reversed);
    patterns.push_back(allEqual);
    patterns.push_back(organPipe);
    patterns.push_back(fewUniques);
    patterns.push_back({});
    patterns.push_back({7});

    for (auto& pattern: patterns)
    {
        auto expected = pattern;
        std::sort(expected.begin(), expected.end());

        EA::Vectors::sort(pattern);
        check(pattern == expected);
    }
};

auto vectorsSortMoveOnly = test("Vectors.sort_move_only_elements") = []
{
    auto v = std::vector<std::unique_ptr<int>> {};

    for (int index = 50; index > 0; --index)
        v.push_back(std::make_unique<int>(index));

    EA::Vectors::sort(v, [](const auto& a, const auto& b) { return *a < *b; });

    for (int index = 0; index < 50; ++index)
        check(*v[(size_t) index] == index + 1);
};

namespace
{
struct Item
{
    int key = 0;
    int payload = 0;

    bool operator==(const Item& other) const = default;
};

int keyOfItem(const Item& item)
{
    return item.key;
}
} // namespace

auto vectorsCountIf = test("Vectors.countIf_counts_every_match") = []
{
    auto v = std::vector<int> {1, 2, 3, 4, 5, 6};

    check(EA::Vectors::countIf(v, [](int value) { return value % 2 == 0; }) == 3);
    check(EA::Vectors::countIf(v, [](int value) { return value > 100; }) == 0);
    check(EA::Vectors::countIf(std::vector<int> {}, [](int) { return true; }) == 0);
};

auto vectorsFindIfMatch = test("Vectors.findIf_returns_the_first_match") = []
{
    auto v = EA::Vector<Item> {{1, 10}, {2, 20}, {2, 30}};

    auto* found = EA::Vectors::findIf(v, [](const Item& i) { return i.key == 2; });

    check(found != nullptr);
    check(found->payload == 20);
};

auto vectorsFindIfNoMatch =
    test("Vectors.findIf_returns_null_when_nothing_matches") = []
{
    auto v = EA::Vector<Item> {{1, 10}};

    check(EA::Vectors::findIf(v, [](const Item& i) { return i.key == 9; })
          == nullptr);
};

auto vectorsFindIfWritesThrough =
    test("Vectors.findIf_is_mutable_on_a_mutable_container") = []
{
    auto v = EA::Vector<Item> {{1, 10}, {2, 20}};

    EA::Vectors::findIf(v, [](const Item& i) { return i.key == 2; })->payload = 99;

    check(v[1].payload == 99);
};

auto vectorsLowerBound = test("Vectors.lowerBoundIndex_lands_before_equal_keys") = []
{
    auto v = std::vector<int> {0, 2, 2, 2, 4};

    check(EA::Vectors::lowerBoundIndex(v, -1) == 0);
    check(EA::Vectors::lowerBoundIndex(v, 0) == 0);
    check(EA::Vectors::lowerBoundIndex(v, 1) == 1);
    check(EA::Vectors::lowerBoundIndex(v, 2) == 1);
    check(EA::Vectors::lowerBoundIndex(v, 3) == 4);
    check(EA::Vectors::lowerBoundIndex(v, 4) == 4);
    check(EA::Vectors::lowerBoundIndex(v, 5) == 5);
    check(EA::Vectors::lowerBoundIndex(std::vector<int> {}, 5) == 0);
};

auto vectorsUpperBound = test("Vectors.upperBoundIndex_lands_after_equal_keys") = []
{
    auto v = std::vector<int> {0, 2, 2, 2, 4};

    check(EA::Vectors::upperBoundIndex(v, -1) == 0);
    check(EA::Vectors::upperBoundIndex(v, 0) == 1);
    check(EA::Vectors::upperBoundIndex(v, 1) == 1);
    check(EA::Vectors::upperBoundIndex(v, 2) == 4);
    check(EA::Vectors::upperBoundIndex(v, 4) == 5);
    check(EA::Vectors::upperBoundIndex(v, 5) == 5);
    check(EA::Vectors::upperBoundIndex(std::vector<int> {}, 5) == 0);
};

auto vectorsBoundsWithKey = test("Vectors.bounds_search_by_a_projected_key") = []
{
    auto v = EA::Vector<Item> {{0, 0}, {2, 1}, {2, 2}, {4, 3}};

    check(EA::Vectors::lowerBoundIndex(v, 2, keyOfItem) == 1);
    check(EA::Vectors::upperBoundIndex(v, 2, keyOfItem) == 3);
    check(EA::Vectors::lowerBoundIndex(v, 3, keyOfItem) == 3);
};

auto vectorsBoundsMatchLinearScan =
    test("Vectors.bounds_agree_with_a_linear_scan") = []
{
    auto v = std::vector<int> {};

    for (int index = 0; index < 200; ++index)
        v.push_back(index / 3);

    for (int value = -1; value < 70; ++value)
    {
        auto lower = 0;
        while (lower < (int) v.size() && v[(size_t) lower] < value)
            ++lower;

        auto upper = 0;
        while (upper < (int) v.size() && !(value < v[(size_t) upper]))
            ++upper;

        check(EA::Vectors::lowerBoundIndex(v, value) == lower);
        check(EA::Vectors::upperBoundIndex(v, value) == upper);
    }
};

auto vectorsInsertSorted =
    test("Vectors.insertSorted_keeps_the_container_sorted") = []
{
    auto v = EA::Vector<int> {};

    for (auto value: {5, 1, 4, 1, 3})
        v.insertSorted(value);

    check(v == EA::Vector<int> {1, 1, 3, 4, 5});
};

auto vectorsInsertSortedIndex =
    test("Vectors.insertSorted_returns_where_it_landed") = []
{
    auto v = EA::Vector<int> {1, 3, 5};

    check(v.insertSorted(0) == 0);
    check(v.insertSorted(9) == 3 + 1);
    check(v.insertSorted(4) == 3);
};

auto vectorsInsertSortedStable =
    test("Vectors.insertSorted_puts_equal_elements_last") = []
{
    auto v = EA::Vector<Item> {};
    auto byKey = [](const Item& a, const Item& b) { return a.key < b.key; };

    v.insertSorted({1, 10}, byKey);
    v.insertSorted({1, 20}, byKey);
    v.insertSorted({1, 30}, byKey);

    check(v[0].payload == 10);
    check(v[1].payload == 20);
    check(v[2].payload == 30);
};

auto vectorsInsertSortedStdVector = test("Vectors.insertSorted_on_std_vector") = []
{
    auto v = std::vector<int> {1, 3};

    check(EA::Vectors::insertSorted(v, 2) == 1);
    check(v == std::vector<int> {1, 2, 3});
};

auto vectorMemberPredicateHelpers = test("Vector.predicate_helper_members") = []
{
    auto v = EA::Vector<Item> {{1, 10}, {2, 20}, {2, 30}};

    check(v.getIndexIf([](const Item& i) { return i.key == 2; }) == 1);
    check(v.countIf([](const Item& i) { return i.key == 2; }) == 2);
    check(v.findIf([](const Item& i) { return i.key == 2; })->payload == 20);
    check(v.lowerBoundIndex(2, keyOfItem) == 1);
    check(v.upperBoundIndex(2, keyOfItem) == 3);
};

//The search and reverse helpers are constexpr, which is what lets Array's own
//members be. They work on any container usable at compile time, std::array
//included, so the property is tested here rather than only through Array.
auto vectorsSearchHelpersAreConstexpr =
    test("Vectors.search_helpers_are_constexpr") = []
{
    constexpr auto values = std::array {1, 2, 3, 4};

    static_assert(EA::Vectors::getIndexOf(values, 3) == 2);
    static_assert(EA::Vectors::getIndexOf(values, 99) == -1);
    static_assert(EA::Vectors::getIndexOfReverse(values, 4) == 3);
    static_assert(EA::Vectors::contains(values, 1));
    static_assert(!EA::Vectors::contains(values, 0));
    static_assert(EA::Vectors::getIndexIf(values, [](int v) { return v > 2; }) == 2);
    static_assert(EA::Vectors::countIf(values, [](int v) { return v > 2; }) == 2);
    static_assert(
        EA::Vectors::getIndexOfComparison(values, [](int v) { return v == 2; })
        == 1);

    static_assert(
        []
        {
            auto local = std::array {1, 2, 3};

            return *EA::Vectors::find(local, 2) == 2
                   && EA::Vectors::find(local, 9) == nullptr
                   && *EA::Vectors::findIf(local, [](int v) { return v == 3; }) == 3
                   && EA::Vectors::getElementRef(local, 1) == 1;
        }());

    check(true);
};

auto vectorsReverseIsConstexpr = test("Vectors.reverse_is_constexpr") = []
{
    constexpr auto reversed = []
    {
        auto values = std::array {1, 2, 3, 4};
        EA::Vectors::reverse(values);

        return values;
    }();

    static_assert(reversed == std::array {4, 3, 2, 1});

    constexpr auto rotated = []
    {
        auto values = std::array {1, 2, 3, 4, 5};
        EA::Vectors::rotate(values.begin(), values.begin() + 2, values.end());

        return values;
    }();

    static_assert(rotated == std::array {3, 4, 5, 1, 2});

    check(true);
};
