#include <Helpers/OperationTracker.h>
#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/StaticVector.h>

using namespace nano;
using EA::TestHelpers::OperationTracker;

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

//Lifecycle tests using the OperationTracker helper: assert that the container
//constructs/destructs/copies/moves its elements the correct number of times.

auto staticVecDestructorCleansUp =
    test("StaticVector.destructor_destroys_all_elements") = []
{
    OperationTracker::reset();
    {
        auto v = EA::StaticVector<OperationTracker, 4>();
        v.emplace_back(1);
        v.emplace_back(2);
        v.emplace_back(3);
        check(OperationTracker::counters.live() == 3);
    }
    check(OperationTracker::counters.live() == 0);
    check(OperationTracker::counters.destructions == 3);
};

auto staticVecClearDestroys = test("StaticVector.clear_destroys_all_elements") = []
{
    OperationTracker::reset();
    auto v = EA::StaticVector<OperationTracker, 4>();
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);

    auto dtorsBefore = OperationTracker::counters.destructions;
    v.clear();
    check(OperationTracker::counters.destructions - dtorsBefore == 3);
    check(v.empty());
};

auto staticVecEmplaceNoCopyOrMove =
    test("StaticVector.emplace_back_forwards_without_copies_or_moves") = []
{
    OperationTracker::reset();
    auto v = EA::StaticVector<OperationTracker, 4>();
    v.emplace_back(42);
    check(OperationTracker::counters.valueConstructions == 1);
    check(OperationTracker::counters.copyConstructions == 0);
    check(OperationTracker::counters.moveConstructions == 0);
    check(v[0].getValue() == 42);
};

auto staticVecAddLvalueCopies = test("StaticVector.add_lvalue_copies") = []
{
    auto t = OperationTracker(7);
    auto v = EA::StaticVector<OperationTracker, 4>();
    OperationTracker::reset();
    v.add(t);
    check(OperationTracker::counters.copyConstructions == 1);
    check(OperationTracker::counters.moveConstructions == 0);
    check(v[0].getValue() == 7);
};

auto staticVecAddRvalueMoves = test("StaticVector.add_rvalue_moves") = []
{
    auto t = OperationTracker(9);
    auto v = EA::StaticVector<OperationTracker, 4>();
    OperationTracker::reset();
    v.add(std::move(t));
    check(OperationTracker::counters.moveConstructions == 1);
    check(OperationTracker::counters.copyConstructions == 0);
    check(v[0].getValue() == 9);
};

auto staticVecPopBackDestroysOne = test("StaticVector.pop_back_destroys_one") = []
{
    OperationTracker::reset();
    auto v = EA::StaticVector<OperationTracker, 4>();
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);

    auto dtorsBefore = OperationTracker::counters.destructions;
    v.pop_back();
    check(OperationTracker::counters.destructions - dtorsBefore == 1);
    check(v.size() == 2);
    check(OperationTracker::counters.live() == 2);
};

auto staticVecRemoveAtReducesLive =
    test("StaticVector.removeAt_reduces_live_count") = []
{
    OperationTracker::reset();
    auto v = EA::StaticVector<OperationTracker, 4>();
    v.emplace_back(10);
    v.emplace_back(20);
    v.emplace_back(30);

    v.removeAt(0);
    check(v.size() == 2);
    check(v[0].getValue() == 20);
    check(v[1].getValue() == 30);
    check(OperationTracker::counters.live() == 2);
};

auto staticVecEraseIfReducesLive =
    test("StaticVector.eraseIf_reduces_live_count") = []
{
    OperationTracker::reset();
    auto v = EA::StaticVector<OperationTracker, 8>();

    for (int i = 1; i <= 5; ++i)
        v.emplace_back(i);

    v.eraseIf([](const OperationTracker& t) { return t.getValue() % 2 == 0; });
    check(v.size() == 3);
    check(OperationTracker::counters.live() == 3);
    check(v[0].getValue() == 1);
    check(v[1].getValue() == 3);
    check(v[2].getValue() == 5);
};

auto staticVecCopyCtorCopiesElements =
    test("StaticVector.copy_ctor_uses_copy_construction") = []
{
    auto src = EA::StaticVector<OperationTracker, 4>();
    src.emplace_back(1);
    src.emplace_back(2);
    src.emplace_back(3);

    OperationTracker::reset();
    auto dst = src;
    check(OperationTracker::counters.copyConstructions == 3);
    check(OperationTracker::counters.moveConstructions == 0);
    check(dst.size() == 3);
    check(dst[0].getValue() == 1);
    check(dst[2].getValue() == 3);
};

auto staticVecCopyAssignClearsAndCopies =
    test("StaticVector.copy_assignment_clears_then_copies") = []
{
    auto src = EA::StaticVector<OperationTracker, 4>();
    src.emplace_back(1);
    src.emplace_back(2);

    auto dst = EA::StaticVector<OperationTracker, 4>();
    dst.emplace_back(10);
    dst.emplace_back(20);
    dst.emplace_back(30);

    OperationTracker::reset();
    dst = src;
    check(OperationTracker::counters.destructions == 3);
    check(OperationTracker::counters.copyConstructions == 2);
    check(dst.size() == 2);
    check(dst[0].getValue() == 1);
    check(dst[1].getValue() == 2);
};

auto staticVecResizeGrowDefaultConstructs =
    test("StaticVector.resize_grow_default_constructs") = []
{
    OperationTracker::reset();
    auto v = EA::StaticVector<OperationTracker, 4>();
    v.resize(3);
    check(OperationTracker::counters.defaultConstructions == 3);
    check(OperationTracker::counters.live() == 3);
    check(v.size() == 3);
};

auto staticVecResizeShrinkDestroys = test("StaticVector.resize_shrink_destroys") = []
{
    OperationTracker::reset();
    auto v = EA::StaticVector<OperationTracker, 8>();
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);

    auto dtorsBefore = OperationTracker::counters.destructions;
    v.resize(1);
    check(OperationTracker::counters.destructions - dtorsBefore == 2);
    check(v.size() == 1);
    check(OperationTracker::counters.live() == 1);
    check(v[0].getValue() == 1);
};

auto staticVecResizeGrowPreservesExisting =
    test("StaticVector.resize_grow_preserves_existing_elements") = []
{
    auto v = EA::StaticVector<OperationTracker, 8>();
    v.emplace_back(1);
    v.emplace_back(2);

    OperationTracker::reset();
    v.resize(4);
    //Only the two new slots should be default-constructed; the existing
    //elements must not be re-created.
    check(OperationTracker::counters.defaultConstructions == 2);
    check(OperationTracker::counters.destructions == 0);
    check(v.size() == 4);
    check(v[0].getValue() == 1);
    check(v[1].getValue() == 2);
};

auto staticVecAddBeyondCapacityDoesntTouch =
    test("StaticVector.add_beyond_capacity_does_not_construct") = []
{
    auto v = EA::StaticVector<OperationTracker, 2>();
    v.emplace_back(1);
    v.emplace_back(2);

    auto t = OperationTracker(3);
    OperationTracker::reset();
    v.add(t);
    check(OperationTracker::counters.copyConstructions == 0);
    check(OperationTracker::counters.moveConstructions == 0);
    check(v.size() == 2);
};
