#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Utilities/TupleUtilities.h>
#include <tuple>
#include <string>
#include <type_traits>

using namespace nano;

namespace
{
struct TestBase
{
    virtual ~TestBase() = default;
};

struct DerivedA : TestBase
{
    int value = 1;
};

struct DerivedB : TestBase
{
    int value = 2;
};

struct DerivedC : TestBase
{
};

struct MembersHolder
{
    int a = 1;
    double b = 2.0;

    auto getMembers() { return std::tie(a, b); }
};
} // namespace

auto tuplesForEachVisitsAll = test("Tuples.for_each_visits_all") = []
{
    auto t = std::make_tuple(1, 2, 3);
    auto sum = 0;
    EA::Tuples::for_each(t, [&](auto x) { sum += x; });
    check(sum == 6);
};

auto tuplesContainerGetByIndex = test("Tuples.Container_get_by_index") = []
{
    auto c = EA::Tuples::Container<int, double>();
    c.get<0>() = 42;
    c.get<1>() = 3.5;
    check(c.get<0>() == 42);
    check(c.get<1>() == 3.5);
};

auto tuplesContainerGetByType = test("Tuples.Container_get_by_type") = []
{
    auto c = EA::Tuples::Container<int, double>();
    c.get<int>() = 7;
    c.get<double>() = 2.5;
    check(c.get<int>() == 7);
    check(c.get<double>() == 2.5);
};

auto tuplesCallForAllTypes = test("Tuples.callForAllTypes_iterates_all_types") = []
{
    auto count = 0;
    EA::Tuples::callForAllTypes<int, double, char>([&](auto*) { ++count; });
    check(count == 3);
};

auto mixedTypesForEachWalksArgs = test("MixedTypes.forEach_visits_each_arg") = []
{
    auto count = 0;
    MixedTypes::forEach([&](auto) { ++count; }, 1, 2.0, 'c');
    check(count == 3);
};

auto tuplesForEachMutatesByReference =
    test("Tuples.for_each_mutates_by_reference") = []
{
    auto t = std::make_tuple(1, 2, 3);
    EA::Tuples::for_each(t, [](auto& x) { x *= 2; });
    check(std::get<0>(t) == 2);
    check(std::get<1>(t) == 4);
    check(std::get<2>(t) == 6);
};

auto tuplesForEachHeterogeneous = test("Tuples.for_each_heterogeneous_tuple") = []
{
    auto t = std::make_tuple(1, 2.5, std::string("a"));
    EA::Tuples::for_each(t,
                         [](auto& x)
                         {
                             using Type = std::decay_t<decltype(x)>;

                             if constexpr (std::is_same_v<Type, std::string>)
                                 x += "b";
                             else
                                 x += 1;
                         });
    check(std::get<0>(t) == 2);
    check(std::get<1>(t) == 3.5);
    check(std::get<2>(t) == "ab");
};

auto tuplesForEachReturnsFunctor = test("Tuples.for_each_returns_functor") = []
{
    struct Accumulator
    {
        int sum = 0;
        void operator()(int x) { sum += x; }
    };

    auto t = std::make_tuple(1, 2, 3);
    auto result = EA::Tuples::for_each(t, Accumulator {});
    check(result.sum == 6);
};

auto tuplesForEachEmptyTuple = test("Tuples.for_each_empty_tuple") = []
{
    auto t = std::tuple<>();
    auto called = false;
    EA::Tuples::for_each(t, [&](auto&) { called = true; });
    check(!called);
};

auto tuplesForEachOverGetMembers = test("Tuples.forEach_uses_getMembers") = []
{
    auto holder = MembersHolder();
    EA::Tuples::forEach(holder, [](auto& member) { member += 1; });
    check(holder.a == 2);
    check(holder.b == 3.0);
};

auto tuplesContainerForEach = test("Tuples.Container_forEach_visits_members") = []
{
    auto c = EA::Tuples::Container<int, double>();
    c.get<int>() = 1;
    c.get<double>() = 2.5;

    auto sum = 0.0;
    c.forEach([&](auto x) { sum += x; });
    check(sum == 3.5);
};

auto tuplesContainerConstAccess = test("Tuples.Container_const_access") = []
{
    auto c = EA::Tuples::Container<int, double>();
    c.get<int>() = 5;
    c.get<double>() = 1.5;

    const auto& constRef = c;
    check(constRef.get<int>() == 5);
    check(constRef.get<0>() == 5);

    auto sum = 0.0;
    constRef.forEach([&](auto x) { sum += x; });
    check(sum == 6.5);
};

auto tuplesMakePointerTuple = test("Tuples.make_pointer_tuple_null_initialized") = []
{
    auto t = EA::Tuples::make_pointer_tuple<int, double>();
    static_assert(std::is_same_v<decltype(t), std::tuple<int*, double*>>);
    check(std::get<0>(t) == nullptr);
    check(std::get<1>(t) == nullptr);
};

auto tuplesCallIfTypeMatchingMatches =
    test("Tuples.callIfTypeMatching_dispatches_to_dynamic_type") = []
{
    auto derived = DerivedB();
    TestBase& base = derived;

    auto matched = 0;
    EA::Tuples::callIfTypeMatching<DerivedA, DerivedB>(
        base, [&](auto& obj) { matched = obj.value; });
    check(matched == 2);
};

auto tuplesCallIfTypeMatchingNoMatch =
    test("Tuples.callIfTypeMatching_skips_unlisted_types") = []
{
    auto derived = DerivedC();
    TestBase& base = derived;

    auto called = false;
    EA::Tuples::callIfTypeMatching<DerivedA, DerivedB>(
        base, [&](auto&) { called = true; });
    check(!called);
};
