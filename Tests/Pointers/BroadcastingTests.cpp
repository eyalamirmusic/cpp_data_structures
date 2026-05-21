#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Pointers/Broadcasting.h>
#include <string>

using namespace nano;

using Mode = EA::Listener::Modes;

auto broadcastingDefaultConstructsValue =
    test("Broadcasting.default_constructs_underlying_value") = []
{
    EA::Broadcasting<int> value;
    check(value.get() == 0);
};

auto broadcastingInitialValueDoesNotFire =
    test("Broadcasting.initial_value_does_not_fire") = []
{
    EA::Broadcasting<int> value {5};
    auto called = 0;
    EA::Listener listener {value, [&] { ++called; }, Mode::TriggerOnEvent};

    check(value.get() == 5);
    check(called == 0);
};

auto broadcastingAssignmentTriggers =
    test("Broadcasting.assignment_triggers_listener") = []
{
    EA::Broadcasting<int> value;
    auto called = 0;
    EA::Listener listener {value, [&] { ++called; }, Mode::TriggerOnEvent};

    value = 5;
    check(called == 1);
    check(value.get() == 5);
};

auto broadcastingAssignmentWithoutChangeIsNoop =
    test("Broadcasting.assignment_without_change_does_not_trigger") = []
{
    EA::Broadcasting<int> value {5};
    auto called = 0;
    EA::Listener listener {value, [&] { ++called; }, Mode::TriggerOnEvent};

    value = 5;
    check(called == 0);

    value = 6;
    check(called == 1);

    value = 6;
    check(called == 1);
};

auto broadcastingCompoundAssignmentTriggers =
    test("Broadcasting.compound_assignment_triggers") = []
{
    EA::Broadcasting<int> value {10};
    auto called = 0;
    EA::Listener listener {value, [&] { ++called; }, Mode::TriggerOnEvent};

    value += 1;
    check(called == 1);
    check(value.get() == 11);

    value -= 5;
    check(called == 2);
    check(value.get() == 6);

    value *= 2;
    check(called == 3);
    check(value.get() == 12);

    value /= 3;
    check(called == 4);
    check(value.get() == 4);
};

auto broadcastingIncrementDecrementTrigger =
    test("Broadcasting.increment_and_decrement_trigger") = []
{
    EA::Broadcasting<int> value {0};
    auto called = 0;
    EA::Listener listener {value, [&] { ++called; }, Mode::TriggerOnEvent};

    ++value;
    check(called == 1);
    check(value.get() == 1);

    --value;
    check(called == 2);
    check(value.get() == 0);
};

auto broadcastingImplicitConvertsToT =
    test("Broadcasting.implicit_converts_to_T") = []
{
    EA::Broadcasting<int> value {7};
    int copy = value;
    check(copy == 7);
    check(value == 7);
    check(7 == value);
};

auto broadcastingWorksWithClassType =
    test("Broadcasting.works_with_class_type_and_arrow") = []
{
    EA::Broadcasting<std::string> text {"hello"};
    auto called = 0;
    EA::Listener listener {text, [&] { ++called; }, Mode::TriggerOnEvent};

    check(text->size() == 5);

    text = std::string("world");
    check(called == 1);
    check(text.get() == "world");

    text = std::string("world");
    check(called == 1);
};

auto broadcastingCopyAssignmentTriggersOnChange =
    test("Broadcasting.copy_assignment_triggers_only_on_change") = []
{
    EA::Broadcasting<int> a {1};
    EA::Broadcasting<int> b {2};
    auto aCalls = 0;
    EA::Listener listener {a, [&] { ++aCalls; }, Mode::TriggerOnEvent};

    a = b;
    check(aCalls == 1);
    check(a.get() == 2);

    a = b;
    check(aCalls == 1);
};

auto broadcastingCopyCtorDoesNotCarryListeners =
    test("Broadcasting.copy_ctor_does_not_carry_listeners") = []
{
    EA::Broadcasting<int> source {3};
    auto sourceCalls = 0;
    EA::Listener listener {source, [&] { ++sourceCalls; }, Mode::TriggerOnEvent};

    EA::Broadcasting<int> copy {source};
    check(copy.get() == 3);

    copy = 100;
    check(sourceCalls == 0);

    source = 200;
    check(sourceCalls == 1);
};

auto broadcastingListenerViaGetBroadcaster =
    test("Broadcasting.listener_via_getBroadcaster") = []
{
    EA::Broadcasting<int> value;
    auto called = 0;
    EA::Listener listener {
        value.getBroadcaster(), [&] { ++called; }, Mode::TriggerOnEvent};

    value = 42;
    check(called == 1);
};

auto broadcastingRefMutatesUnderlyingAndTriggers =
    test("BroadcastingRef.mutates_underlying_and_triggers") = []
{
    int storage = 0;
    EA::BroadcastingRef<int> view {storage};
    auto called = 0;
    EA::Listener listener {view, [&] { ++called; }, Mode::TriggerOnEvent};

    view = 9;
    check(called == 1);
    check(storage == 9);
    check(view.get() == 9);
};

auto broadcastingRefReflectsExternalChanges =
    test("BroadcastingRef.reflects_external_changes_through_get") = []
{
    int storage = 1;
    EA::BroadcastingRef<int> view {storage};
    check(view.get() == 1);

    storage = 42;
    check(view.get() == 42);
};

auto broadcastingRefNoTriggerOnSameValue =
    test("BroadcastingRef.no_trigger_when_assigned_same_value") = []
{
    int storage = 5;
    EA::BroadcastingRef<int> view {storage};
    auto called = 0;
    EA::Listener listener {view, [&] { ++called; }, Mode::TriggerOnEvent};

    view = 5;
    check(called == 0);

    view = 6;
    check(called == 1);
    check(storage == 6);
};

auto broadcastingRefCompoundAssignmentTriggers =
    test("BroadcastingRef.compound_assignment_triggers") = []
{
    int storage = 10;
    EA::BroadcastingRef<int> view {storage};
    auto called = 0;
    EA::Listener listener {view, [&] { ++called; }, Mode::TriggerOnEvent};

    view += 5;
    check(called == 1);
    check(storage == 15);

    ++view;
    check(called == 2);
    check(storage == 16);
};

auto broadcastingRefCopyAssignmentAssignsValue =
    test("BroadcastingRef.copy_assignment_assigns_underlying_value") = []
{
    int aStorage = 1;
    int bStorage = 2;
    EA::BroadcastingRef<int> a {aStorage};
    EA::BroadcastingRef<int> b {bStorage};
    auto aCalls = 0;
    EA::Listener listener {a, [&] { ++aCalls; }, Mode::TriggerOnEvent};

    a = b;
    check(aCalls == 1);
    check(aStorage == 2);
    check(bStorage == 2);
};
