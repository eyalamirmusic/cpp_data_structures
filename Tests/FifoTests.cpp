#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>

using namespace nano;

auto fifoPushPull = test("Fifo.push_then_pull_returns_pushed_value") = []
{
    auto fifo = EA::Fifo<int, 5>();
    fifo.fill(0);
    fifo.push(42);
    check(fifo.pull() == 42);
};

auto fifoPullMostRecent = test("Fifo.pull_returns_most_recent_push") = []
{
    auto fifo = EA::Fifo<int, 5>();
    fifo.fill(0);
    fifo.push(1);
    fifo.push(2);
    fifo.push(3);
    check(fifo.pull() == 3);
};

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
