#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/SharedGUIData.h>

using namespace nano;

auto guiToRealTimePushGet = test("GUIToRealTime.push_visible_from_realtime") = []
{
    auto g = EA::GUIToRealTime<int, 5>();
    g.push(42);
    check(g.getRealTime() == 42);
};

auto guiToRealTimeFill = test("GUIToRealTime.fill") = []
{
    auto g = EA::GUIToRealTime<int, 5>();
    g.fill(7);
    check(g.getRealTime() == 7);
};

auto sharedWithRTPushAndRead = test("SharedWithRealTime.push_publishes_value") = []
{
    auto shared = EA::SharedWithRealTime<int, 5>();
    *shared = 99;
    shared.push();
    shared.blockStarted();
    check(shared.getRealTime() == 99);
};

auto realTimeToGUIFlag = test("RealTimeToGUI.update_flag_bumps_on_push") = []
{
    auto r = EA::RealTimeToGUI<int, 5>();
    auto before = r.updateFlag.load();
    r.push(5);
    auto after = r.updateFlag.load();
    check(after > before);
};

auto guiToRealTimeTryPush = test("GUIToRealTime.tryPush_fails_when_full") = []
{
    auto g = EA::GUIToRealTime<int, 5>(0);
    check(g.getRealTime() == 0);

    check(g.tryPush(1));
    check(g.tryPush(2));
    check(g.tryPush(3));
    check(!g.tryPush(4));

    //The dropped value doesn't corrupt anything - the realtime side
    //still sees the most recent successfully pushed value
    check(g.getRealTime() == 3);
};

auto sharedWithRTTryPush = test("SharedWithRealTime.tryPush_fails_when_full") = []
{
    auto shared = EA::SharedWithRealTime<int, 5>();
    shared.blockStarted();

    *shared = 1;
    check(shared.tryPush());
    *shared = 2;
    check(shared.tryPush());
    *shared = 3;
    check(shared.tryPush());
    *shared = 4;
    check(!shared.tryPush());

    check(shared.getRealTime() == 3);
};

auto realTimeToGUITryPush = test("RealTimeToGUI.tryPush_full_skips_flag_update") = []
{
    auto r = EA::RealTimeToGUI<int, 5>();
    r.push(1);
    check(r.pull() == 1);

    check(r.tryPush(2));
    check(r.tryPush(3));
    check(r.tryPush(4));

    auto before = r.updateFlag.load();
    check(!r.tryPush(5));
    check(r.updateFlag.load() == before);

    check(r.pull() == 4);
};
