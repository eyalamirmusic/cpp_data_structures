#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/SharedGUIData.h>

using namespace nano;

auto guiToRealTimePushGet =
    test("GUIToRealTime.push_visible_from_realtime") = []
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

auto sharedWithRTPushAndRead =
    test("SharedWithRealTime.push_publishes_value") = []
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
