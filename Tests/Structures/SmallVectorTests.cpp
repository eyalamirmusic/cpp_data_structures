#include <Helpers/OperationTracker.h>
#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/SmallVector.h>

using namespace nano;
using EA::TestHelpers::OperationTracker;

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

//Lifecycle tests using the OperationTracker helper: assert that the container
//constructs/destructs/copies/moves its elements correctly in both static and
//dynamic modes, and that the static->dynamic transition moves (not copies).

auto smallVecDestructorCleansUpStatic =
    test("SmallVector.destructor_destroys_all_static_elements") = []
{
    OperationTracker::reset();
    {
        auto v = EA::SmallVector<OperationTracker, 4>();
        v.emplace_back(1);
        v.emplace_back(2);
        v.emplace_back(3);
        check(v.isStatic());
        check(OperationTracker::counters.live() == 3);
    }
    check(OperationTracker::counters.live() == 0);
};

auto smallVecDestructorCleansUpDynamic =
    test("SmallVector.destructor_destroys_all_dynamic_elements") = []
{
    OperationTracker::reset();
    {
        auto v = EA::SmallVector<OperationTracker, 2>();
        v.emplace_back(1);
        v.emplace_back(2);
        v.emplace_back(3);
        v.emplace_back(4);
        check(!v.isStatic());
        check(OperationTracker::counters.live() == 4);
    }
    check(OperationTracker::counters.live() == 0);
};

auto smallVecEmplaceStaticNoCopy =
    test("SmallVector.emplace_back_static_forwards_without_copies") = []
{
    OperationTracker::reset();
    auto v = EA::SmallVector<OperationTracker, 4>();
    v.emplace_back(42);
    check(v.isStatic());
    check(OperationTracker::counters.valueConstructions == 1);
    check(OperationTracker::counters.copyConstructions == 0);
    check(OperationTracker::counters.moveConstructions == 0);
    check(v[0].getValue() == 42);
};

auto smallVecSwitchMovesNotCopies =
    test("SmallVector.switch_to_dynamic_moves_not_copies") = []
{
    auto v = EA::SmallVector<OperationTracker, 3>();
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);
    check(v.isStatic());

    OperationTracker::reset();
    v.emplace_back(4);

    check(!v.isStatic());
    check(OperationTracker::counters.copyConstructions == 0);
    check(OperationTracker::counters.moveConstructions >= 3);
    check(v.size() == 4);
    check(v[0].getValue() == 1);
    check(v[3].getValue() == 4);
};

auto smallVecSwitchDestroysOldStaticStorage =
    test("SmallVector.switch_destroys_old_static_storage") = []
{
    auto v = EA::SmallVector<OperationTracker, 2>();
    v.emplace_back(1);
    v.emplace_back(2);

    auto countersBefore = OperationTracker::counters;
    v.emplace_back(3);
    check(!v.isStatic());
    //Expect the two old static slots to be destroyed as part of the switch.
    check(OperationTracker::counters.destructions - countersBefore.destructions
          >= 2);
    check(v.size() == 3);
    check(v[0].getValue() == 1);
    check(v[1].getValue() == 2);
    check(v[2].getValue() == 3);
};

auto smallVecClearDynamicDestroys =
    test("SmallVector.clear_destroys_all_dynamic_elements") = []
{
    auto v = EA::SmallVector<OperationTracker, 2>();
    for (int i = 1; i <= 5; ++i)
        v.emplace_back(i);
    check(!v.isStatic());

    OperationTracker::reset();
    v.clear();
    check(OperationTracker::counters.destructions >= 5);
    check(v.empty());
};

auto smallVecPopBackDestroysOne = test("SmallVector.pop_back_destroys_one") = []
{
    OperationTracker::reset();
    auto v = EA::SmallVector<OperationTracker, 4>();
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);

    auto dtorsBefore = OperationTracker::counters.destructions;
    v.pop_back();
    check(OperationTracker::counters.destructions - dtorsBefore == 1);
    check(v.size() == 2);
};

auto smallVecEraseIfDynamic =
    test("SmallVector.eraseIf_destroys_matching_in_dynamic") = []
{
    OperationTracker::reset();
    auto v = EA::SmallVector<OperationTracker, 2>();
    for (int i = 1; i <= 5; ++i)
        v.emplace_back(i);
    check(!v.isStatic());

    v.eraseIf([](const OperationTracker& t) { return t.getValue() % 2 == 0; });
    check(v.size() == 3);
    check(OperationTracker::counters.live() == 3);
    check(v[0].getValue() == 1);
    check(v[1].getValue() == 3);
    check(v[2].getValue() == 5);
};

auto smallVecAddLvalueStaticCopies =
    test("SmallVector.add_lvalue_static_copies") = []
{
    auto t = OperationTracker(5);
    auto v = EA::SmallVector<OperationTracker, 4>();
    OperationTracker::reset();
    v.add(t);
    check(v.isStatic());
    check(OperationTracker::counters.copyConstructions == 1);
    check(OperationTracker::counters.moveConstructions == 0);
    check(v[0].getValue() == 5);
};

auto smallVecAddRvalueStaticMoves = test("SmallVector.add_rvalue_static_moves") = []
{
    auto t = OperationTracker(5);
    auto v = EA::SmallVector<OperationTracker, 4>();
    OperationTracker::reset();
    v.add(std::move(t));
    check(v.isStatic());
    check(OperationTracker::counters.moveConstructions == 1);
    check(OperationTracker::counters.copyConstructions == 0);
    check(v[0].getValue() == 5);
};

auto smallVecCopyCtorCopiesStatic =
    test("SmallVector.copy_ctor_in_static_mode_uses_copies") = []
{
    auto src = EA::SmallVector<OperationTracker, 4>();
    src.emplace_back(1);
    src.emplace_back(2);
    check(src.isStatic());

    OperationTracker::reset();
    auto dst = src;
    check(dst.isStatic());
    check(OperationTracker::counters.copyConstructions == 2);
    check(OperationTracker::counters.moveConstructions == 0);
    check(dst.size() == 2);
    check(dst[0].getValue() == 1);
    check(dst[1].getValue() == 2);
};

auto smallVecCopyCtorCopiesDynamic =
    test("SmallVector.copy_ctor_in_dynamic_mode_uses_copies") = []
{
    auto src = EA::SmallVector<OperationTracker, 2>();
    for (int i = 1; i <= 5; ++i)
        src.emplace_back(i);
    check(!src.isStatic());

    OperationTracker::reset();
    auto dst = src;
    check(!dst.isStatic());
    check(OperationTracker::counters.copyConstructions >= 5);
    check(OperationTracker::counters.moveConstructions == 0);
    check(dst.size() == 5);
    check(dst[0].getValue() == 1);
    check(dst[4].getValue() == 5);
};

auto smallVecResizeGrowStaticDefaultConstructs =
    test("SmallVector.resize_grow_static_default_constructs") = []
{
    OperationTracker::reset();
    auto v = EA::SmallVector<OperationTracker, 4>();
    v.resize(3);
    check(v.isStatic());
    check(OperationTracker::counters.defaultConstructions == 3);
    check(OperationTracker::counters.live() == 3);
    check(v.size() == 3);
};
