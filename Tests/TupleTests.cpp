#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>
#include <tuple>

using namespace nano;

auto tuplesForEachVisitsAll = test("Tuples.for_each_visits_all") = []
{
    auto t = std::make_tuple(1, 2, 3);
    auto sum = 0;
    EA::Tuples::for_each(t, [&](auto x) { sum += x; });
    check(sum == 6);
};

auto tuplesContainerGetByIndex =
    test("Tuples.Container_get_by_index") = []
{
    auto c = EA::Tuples::Container<int, double>();
    c.get<0>() = 42;
    c.get<1>() = 3.5;
    check(c.get<0>() == 42);
    check(c.get<1>() == 3.5);
};

auto tuplesContainerGetByType =
    test("Tuples.Container_get_by_type") = []
{
    auto c = EA::Tuples::Container<int, double>();
    c.get<int>() = 7;
    c.get<double>() = 2.5;
    check(c.get<int>() == 7);
    check(c.get<double>() == 2.5);
};

auto tuplesCallForAllTypes =
    test("Tuples.callForAllTypes_iterates_all_types") = []
{
    auto count = 0;
    EA::Tuples::callForAllTypes<int, double, char>([&](auto*) { ++count; });
    check(count == 3);
};

auto mixedTypesForEachWalksArgs =
    test("MixedTypes.forEach_visits_each_arg") = []
{
    auto count = 0;
    MixedTypes::forEach([&](auto) { ++count; }, 1, 2.0, 'c');
    check(count == 3);
};
