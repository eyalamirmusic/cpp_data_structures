#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/Fifo.h>

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
