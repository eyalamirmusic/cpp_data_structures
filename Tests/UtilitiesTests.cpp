#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/StaticVector.h>
#include <ea_data_structures/Structures/Vector.h>
#include <ea_data_structures/Utilities/GenericUtilities.h>
#include <ea_data_structures/Utilities/MapUtilities.h>
#include <ea_data_structures/Utilities/StaticObjects.h>
#include <ea_data_structures/Utilities/VectorUtilities.h>
#include <map>
#include <vector>

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
    auto idx = EA::Vectors::getIndexOfComparison(
        v, [](const auto& e) { return e == 15; });
    check(idx == 2);
    auto missing = EA::Vectors::getIndexOfComparison(
        v, [](const auto& e) { return e == 999; });
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
    EA::Vectors::zipWithIndexed(a, b,
                                [&](int, int, int idx) { lastIndex = idx; });
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

auto mapUtilsContains = test("MapUtils.contains") = []
{
    auto m = std::map<int, int> {{1, 10}, {2, 20}};
    check(EA::MapUtils::contains(m, 1));
    check(!EA::MapUtils::contains(m, 99));
};

auto mapUtilsGet = test("MapUtils.get_returns_pointer_or_null") = []
{
    auto m = std::map<int, int> {{1, 10}};
    auto* p = EA::MapUtils::get(m, 1);
    check(p != nullptr);
    check(*p == 10);
    check(EA::MapUtils::get(m, 99) == nullptr);
};

auto mapUtilsGetDefault = test("MapUtils.get_with_default") = []
{
    auto m = std::map<int, int> {{1, 10}};
    check(EA::MapUtils::get(m, 1, -1) == 10);
    check(EA::MapUtils::get(m, 99, -1) == -1);
};

namespace
{
struct StaticThing
{
    int v = 5;
};
} // namespace

auto staticObjectSameRef = test("getStaticObject.returns_same_reference") = []
{
    auto& a = EA::getStaticObject<StaticThing>();
    auto& b = EA::getStaticObject<StaticThing>();
    check(&a == &b);
    check(a.v == 5);
};

auto staticStackObjectSameRef = test("getStaticStackObject.returns_same_ref") = []
{
    auto& a = EA::getStaticStackObject<StaticThing>();
    auto& b = EA::getStaticStackObject<StaticThing>();
    check(&a == &b);
};

auto updateMarkerEquals = test("UpdateMarker.new_markers_are_equal") = []
{
    auto a = EA::UpdateMarker();
    auto b = EA::UpdateMarker();
    check(a == b);
};

auto updateMarkerUpdateDiverges = test("UpdateMarker.update_diverges") = []
{
    auto a = EA::UpdateMarker();
    auto b = EA::UpdateMarker();
    a.update();
    check(!(a == b));
    b.update();
    check(a == b);
};
