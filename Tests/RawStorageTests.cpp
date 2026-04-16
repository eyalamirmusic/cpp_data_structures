#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>

using namespace nano;

static_assert(sizeof(EA::RawStorage<int>) == sizeof(int),
              "RawStorage<T> must be sizeof(T); union collapses dual members");
static_assert(alignof(EA::RawStorage<int>) == alignof(int),
              "RawStorage<T> must have alignof(T)");

namespace
{
struct alignas(16) OverAligned
{
    char bytes[48];
};
} // namespace

static_assert(sizeof(EA::RawStorage<OverAligned>) == sizeof(OverAligned));
static_assert(alignof(EA::RawStorage<OverAligned>) == alignof(OverAligned));

auto rawStorageCreateAndGet = test("RawStorage.create_returns_valid_pointer") = []
{
    auto storage = EA::RawStorage<int>();
    auto* p = storage.create(42);
    check(p != nullptr);
    check(p == storage.get());
    check(*p == 42);
    storage.destroy();
};

auto rawStorageDereference = test("RawStorage.operator_star_and_arrow") = []
{
    struct Point
    {
        int x;
        int y;
    };
    auto storage = EA::RawStorage<Point>();
    storage.create(3, 7);
    check(storage->x == 3);
    check(storage->y == 7);
    check((*storage).x == 3);
    storage.destroy();
};

auto rawStorageForwardsArgs = test("RawStorage.create_forwards_constructor_args") = []
{
    struct Multi
    {
        Multi(int a, int b, int c)
            : sum(a + b + c)
        {
        }
        int sum;
    };
    auto storage = EA::RawStorage<Multi>();
    storage.create(1, 2, 3);
    check(storage->sum == 6);
    storage.destroy();
};

namespace
{
struct RawLifecycleCounter
{
    explicit RawLifecycleCounter(int& counter)
        : count(counter)
    {
        ++count;
    }
    ~RawLifecycleCounter() { --count; }
    int& count;
};
} // namespace

auto rawStorageDestroyCallsDtor = test("RawStorage.destroy_calls_destructor") = []
{
    auto counter = 0;
    auto storage = EA::RawStorage<RawLifecycleCounter>();
    storage.create(counter);
    check(counter == 1);
    storage.destroy();
    check(counter == 0);
};

auto rawStorageAddressAliases =
    test("RawStorage.memory_and_object_share_address") = []
{
    auto storage = EA::RawStorage<int>();
    auto* viaObject = storage.get();
    auto* viaBytes = reinterpret_cast<const int*>(&storage);
    check(viaObject == viaBytes);
};
