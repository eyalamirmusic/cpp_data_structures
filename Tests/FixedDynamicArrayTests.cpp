#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>

using namespace nano;

auto fixedDynSize = test("FixedDynamicArray.size") = []
{
    auto a = EA::FixedDynamicArray<int>(5);
    check(a.size() == 5);
};

auto fixedDynIndexing = test("FixedDynamicArray.indexing") = []
{
    auto a = EA::FixedDynamicArray<int>(3, 42);
    check(a[0] == 42);
    check(a[1] == 42);
    check(a[2] == 42);
};

auto fixedDynWrite = test("FixedDynamicArray.write_through_index") = []
{
    auto a = EA::FixedDynamicArray<int>(3);
    a[0] = 100;
    a[1] = 200;
    a[2] = 300;
    check(a[0] == 100);
    check(a[2] == 300);
};

auto fixedDynIteration = test("FixedDynamicArray.range_based_for") = []
{
    auto a = EA::FixedDynamicArray<int>(4, 7);
    auto count = 0;
    auto sum = 0;
    for (auto v: a)
    {
        sum += v;
        ++count;
    }
    check(count == 4);
    check(sum == 28);
};

auto fixedDynZeroSize = test("FixedDynamicArray.zero_size") = []
{
    auto a = EA::FixedDynamicArray<int>(0);
    check(a.size() == 0);
    check(a.begin() == a.end());
};

namespace
{
struct FixedLifecycleCounter
{
    explicit FixedLifecycleCounter(int& counter)
        : count(counter)
    {
        ++count;
    }
    ~FixedLifecycleCounter() { --count; }
    int& count;
};
} // namespace

auto fixedDynLifecycle =
    test("FixedDynamicArray.constructs_and_destroys_all_elements") = []
{
    auto counter = 0;
    {
        auto a = EA::FixedDynamicArray<FixedLifecycleCounter>(5, counter);
        check(counter == 5);
    }
    check(counter == 0);
};
