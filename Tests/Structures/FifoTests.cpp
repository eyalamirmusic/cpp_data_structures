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

auto fifoOverwritesOldest =
    test("Fifo.push_overwrites_oldest_when_never_pulled") = []
{
    auto fifo = EA::Fifo<int, 5>();
    fifo.fill(0);

    //Without a reader holding a slot, pushing can wrap around freely
    for (auto value = 1; value <= 12; ++value)
        fifo.push(value);

    check(fifo.pull() == 12);
};

auto fifoTryPushFull =
    test("Fifo.tryPush_fails_when_full_and_keeps_newest_value") = []
{
    auto fifo = EA::Fifo<int, 5>();
    fifo.fill(0);

    fifo.push(1);
    check(fifo.pull() == 1);

    check(fifo.tryPush(2));
    check(fifo.tryPush(3));
    check(fifo.tryPush(4));

    //The next write would land on the slot the reader is still holding
    check(!fifo.tryPush(5));

    //The rejected value was dropped without corrupting the fifo:
    //pull() still returns the newest successfully pushed value, not stale data
    check(fifo.pull() == 4);
};

auto fifoTryPushAfterPull = test("Fifo.tryPush_succeeds_again_after_pull") = []
{
    auto fifo = EA::Fifo<int, 5>();
    fifo.fill(0);

    fifo.push(1);
    check(fifo.pull() == 1);

    while (fifo.tryPush(2))
    {
    }

    //Pulling moves the reader's slot forward, making room to push again
    check(fifo.pull() == 2);
    check(fifo.tryPush(3));
    check(fifo.pull() == 3);
};

auto fifoDefaultSize = test("Fifo.default_size_holds_many_pushes_between_pulls") = []
{
    auto fifo = EA::Fifo<int>();
    fifo.fill(0);

    fifo.push(0);
    check(fifo.pull() == 0);

    //Default size of 50 leaves room for 48 pushes between pulls
    for (auto value = 1; value <= 48; ++value)
        check(fifo.tryPush(value));

    check(!fifo.tryPush(49));
    check(fifo.pull() == 48);
};
