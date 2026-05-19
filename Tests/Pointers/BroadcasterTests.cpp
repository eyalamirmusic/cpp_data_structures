#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Pointers/Broadcaster.h>
#include <memory>
#include <optional>

using namespace nano;

using Mode = EA::Listener::Modes;

auto broadcasterTriggersListener = test("Broadcaster.trigger_invokes_listener") = []
{
    EA::Broadcaster broadcaster;
    auto called = 0;
    EA::Listener listener {broadcaster, [&] { ++called; }, Mode::TriggerOnEvent};

    broadcaster.trigger();
    broadcaster.trigger();

    check(called == 2);
};

auto broadcasterTriggersAllListeners =
    test("Broadcaster.trigger_invokes_all_listeners_in_order") = []
{
    EA::Broadcaster broadcaster;
    auto order = 0;
    auto firstAt = -1;
    auto secondAt = -1;
    auto thirdAt = -1;
    EA::Listener a {broadcaster, [&] { firstAt = order++; }, Mode::TriggerOnEvent};
    EA::Listener b {broadcaster, [&] { secondAt = order++; }, Mode::TriggerOnEvent};
    EA::Listener c {broadcaster, [&] { thirdAt = order++; }, Mode::TriggerOnEvent};

    broadcaster.trigger();

    check(firstAt == 0);
    check(secondAt == 1);
    check(thirdAt == 2);
};

auto broadcasterTriggerWithNoListeners =
    test("Broadcaster.trigger_with_no_listeners_is_noop") = []
{
    EA::Broadcaster broadcaster;
    broadcaster.trigger();
    check(true);
};

auto triggerNowFiresOnConstruction =
    test("Listener.trigger_now_fires_on_construction") = []
{
    EA::Broadcaster broadcaster;
    auto called = 0;
    EA::Listener listener {broadcaster, [&] { ++called; }};
    check(called == 1);
};

auto triggerNowIsTheDefaultAndAlsoFiresOnEvent =
    test("Listener.trigger_now_default_also_fires_on_event") = []
{
    EA::Broadcaster broadcaster;
    auto called = 0;
    EA::Listener listener {broadcaster, [&] { ++called; }};

    broadcaster.trigger();
    broadcaster.trigger();

    check(called == 3);
};

auto triggerOnEventSuppressesConstructionFire =
    test("Listener.trigger_on_event_does_not_fire_on_construction") = []
{
    EA::Broadcaster broadcaster;
    auto called = 0;
    EA::Listener listener {broadcaster, [&] { ++called; }, Mode::TriggerOnEvent};

    check(called == 0);

    broadcaster.trigger();
    check(called == 1);
};

auto listenerDestructionStopsCallbacks =
    test("Listener.destruction_stops_callbacks") = []
{
    EA::Broadcaster broadcaster;
    auto called = 0;

    {
        EA::Listener listener {broadcaster, [&] { ++called; }, Mode::TriggerOnEvent};
        broadcaster.trigger();
    }

    broadcaster.trigger();
    check(called == 1);
};

auto listenerOutlivesBroadcaster =
    test("Listener.can_outlive_broadcaster_safely") = []
{
    auto called = 0;
    std::optional<EA::Listener> listener;

    {
        EA::Broadcaster broadcaster;
        listener.emplace(broadcaster, [&] { ++called; }, Mode::TriggerOnEvent);
        broadcaster.trigger();
    }

    listener.reset();
    check(called == 1);
};

auto broadcasterCopyCtorDoesNotCopyListeners =
    test("Broadcaster.copy_ctor_does_not_copy_listeners") = []
{
    EA::Broadcaster source;
    auto sourceCalls = 0;
    EA::Listener listener {source, [&] { ++sourceCalls; }, Mode::TriggerOnEvent};

    EA::Broadcaster copy {source};
    copy.trigger();
    source.trigger();

    check(sourceCalls == 1);
};

auto broadcasterMoveCtorDoesNotMoveListeners =
    test("Broadcaster.move_ctor_does_not_move_listeners") = []
{
    EA::Broadcaster source;
    auto sourceCalls = 0;
    EA::Listener listener {source, [&] { ++sourceCalls; }, Mode::TriggerOnEvent};

    EA::Broadcaster moved {std::move(source)};
    moved.trigger();
    source.trigger();

    check(sourceCalls == 1);
};

auto broadcasterAssignmentDoesNotTouchListeners =
    test("Broadcaster.copy_assign_preserves_existing_listeners") = []
{
    EA::Broadcaster lhs;
    EA::Broadcaster rhs;
    auto lhsCalls = 0;
    auto rhsCalls = 0;
    EA::Listener lhsListener {lhs, [&] { ++lhsCalls; }, Mode::TriggerOnEvent};
    EA::Listener rhsListener {rhs, [&] { ++rhsCalls; }, Mode::TriggerOnEvent};

    lhs = rhs;
    lhs.trigger();
    rhs.trigger();

    check(lhsCalls == 1);
    check(rhsCalls == 1);
};

auto removingListenerDuringTriggerIsSafe =
    test("Broadcaster.removing_listener_during_trigger_is_safe") = []
{
    EA::Broadcaster broadcaster;
    auto firstCalls = 0;
    auto thirdCalls = 0;

    EA::Listener first {broadcaster, [&] { ++firstCalls; }, Mode::TriggerOnEvent};
    std::unique_ptr<EA::Listener> second;
    second = std::make_unique<EA::Listener>(
        broadcaster, [&] { second.reset(); }, Mode::TriggerOnEvent);
    EA::Listener third {broadcaster, [&] { ++thirdCalls; }, Mode::TriggerOnEvent};

    broadcaster.trigger();

    check(firstCalls == 1);
    check(thirdCalls == 1);
    check(second == nullptr);

    broadcaster.trigger();
    check(firstCalls == 2);
    check(thirdCalls == 2);
};

auto addingListenerDuringTriggerSkipsCurrentRound =
    test("Broadcaster.adding_listener_during_trigger_skips_current_round") = []
{
    EA::Broadcaster broadcaster;
    auto firstCalls = 0;
    auto lateCalls = 0;
    std::unique_ptr<EA::Listener> lateListener;

    EA::Listener first {
        broadcaster,
        [&]
        {
            ++firstCalls;
            if (lateListener == nullptr)
            {
                lateListener = std::make_unique<EA::Listener>(
                    broadcaster, [&] { ++lateCalls; }, Mode::TriggerOnEvent);
            }
        },
        Mode::TriggerOnEvent};

    broadcaster.trigger();
    check(firstCalls == 1);
    check(lateCalls == 0);

    broadcaster.trigger();
    check(firstCalls == 2);
    check(lateCalls == 1);
};

auto removingFiringListenerDuringTriggerIsSafe =
    test("Broadcaster.listener_removing_self_during_trigger_is_safe") = []
{
    EA::Broadcaster broadcaster;
    auto firstCalls = 0;
    auto thirdCalls = 0;
    std::unique_ptr<EA::Listener> selfRemoving;

    EA::Listener first {broadcaster, [&] { ++firstCalls; }, Mode::TriggerOnEvent};
    selfRemoving = std::make_unique<EA::Listener>(
        broadcaster, [&] { selfRemoving.reset(); }, Mode::TriggerOnEvent);
    EA::Listener third {broadcaster, [&] { ++thirdCalls; }, Mode::TriggerOnEvent};

    broadcaster.trigger();

    check(firstCalls == 1);
    check(thirdCalls == 1);
    check(selfRemoving == nullptr);

    broadcaster.trigger();
    check(firstCalls == 2);
    check(thirdCalls == 2);
};

auto multipleAddsDuringTriggerAllFireNextRound =
    test("Broadcaster.multiple_adds_during_trigger_all_fire_next_round") = []
{
    EA::Broadcaster broadcaster;
    auto triggerCalls = 0;
    auto firstNewCalls = 0;
    auto secondNewCalls = 0;
    std::unique_ptr<EA::Listener> firstNew;
    std::unique_ptr<EA::Listener> secondNew;

    EA::Listener seed {
        broadcaster,
        [&]
        {
            ++triggerCalls;
            if (firstNew == nullptr)
            {
                firstNew = std::make_unique<EA::Listener>(
                    broadcaster, [&] { ++firstNewCalls; }, Mode::TriggerOnEvent);
                secondNew = std::make_unique<EA::Listener>(
                    broadcaster, [&] { ++secondNewCalls; }, Mode::TriggerOnEvent);
            }
        },
        Mode::TriggerOnEvent};

    broadcaster.trigger();
    check(triggerCalls == 1);
    check(firstNewCalls == 0);
    check(secondNewCalls == 0);

    broadcaster.trigger();
    check(triggerCalls == 2);
    check(firstNewCalls == 1);
    check(secondNewCalls == 1);
};

auto addAndDetachInSameCallbackIsNoop =
    test("Broadcaster.add_then_detach_in_same_callback_is_noop") = []
{
    EA::Broadcaster broadcaster;
    auto seedCalls = 0;
    auto transientCalls = 0;
    auto didTransient = false;

    EA::Listener seed {broadcaster,
                       [&]
                       {
                           ++seedCalls;
                           if (!didTransient)
                           {
                               didTransient = true;
                               EA::Listener transient {broadcaster,
                                                       [&] { ++transientCalls; },
                                                       Mode::TriggerOnEvent};
                           }
                       },
                       Mode::TriggerOnEvent};

    broadcaster.trigger();
    broadcaster.trigger();

    check(seedCalls == 2);
    check(transientCalls == 0);
};

auto removeAndAddInteractAcrossRounds =
    test("Broadcaster.remove_then_add_compacts_between_rounds") = []
{
    EA::Broadcaster broadcaster;
    auto aCalls = 0;
    auto bCalls = 0;

    auto a = std::make_unique<EA::Listener>(
        broadcaster, [&] { ++aCalls; }, Mode::TriggerOnEvent);
    EA::Listener b {broadcaster, [&] { ++bCalls; }, Mode::TriggerOnEvent};

    broadcaster.trigger();
    a.reset();
    broadcaster.trigger();

    EA::Listener c {broadcaster, [&] { ++bCalls; }, Mode::TriggerOnEvent};
    broadcaster.trigger();

    check(aCalls == 1);
    check(bCalls == 1 + 1 + 2);
};
