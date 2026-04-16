#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>
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
