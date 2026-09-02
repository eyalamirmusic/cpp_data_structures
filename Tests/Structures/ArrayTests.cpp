#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/Array.h>
#include <type_traits>

using namespace nano;

auto arraySizeIsInt = test("Array.size_returns_int") = []
{
    auto a = EA::Array<int, 4>();
    static_assert(std::is_same_v<decltype(a.size()), int>);
    check(a.size() == 4);
};

auto arrayContains = test("Array.contains") = []
{
    auto a = EA::Array<int, 3>();
    a[0] = 10;
    a[1] = 20;
    a[2] = 30;
    check(a.contains(20));
    check(!a.contains(5));
};

auto arrayFill = test("Array.fill") = []
{
    auto a = EA::Array<int, 5>();
    a.fill(7);
    for (auto i = 0; i < a.size(); ++i)
        check(a[i] == 7);
};

auto arraySort = test("Array.sort") = []
{
    auto a = EA::Array<int, 4>();
    a[0] = 4;
    a[1] = 2;
    a[2] = 3;
    a[3] = 1;
    a.sort();
    check(a[0] == 1);
    check(a[1] == 2);
    check(a[2] == 3);
    check(a[3] == 4);
};

auto arrayIndexOf = test("Array.getIndexOf") = []
{
    auto a = EA::Array<int, 3>();
    a[0] = 100;
    a[1] = 200;
    a[2] = 300;
    check(a.getIndexOf(200) == 1);
    check(a.getIndexOf(999) == -1);
};

auto arrayInitializerList = test("Array.initializer_list_construction") = []
{
    auto a = EA::Array<int, 4> {1, 2, 3, 4};
    check(a.size() == 4);
    check(a[0] == 1);
    check(a[1] == 2);
    check(a[2] == 3);
    check(a[3] == 4);
};

auto arrayInitializerListPartial =
    test("Array.initializer_list_partial_fills_rest") = []
{
    auto a = EA::Array<int, 5> {1, 2, 3};
    check(a[0] == 1);
    check(a[1] == 2);
    check(a[2] == 3);
    check(a[3] == 0);
    check(a[4] == 0);
};

auto arrayBackFront = test("Array.back_and_front") = []
{
    auto a = EA::Array<int, 3> {10, 20, 30};
    check(a.front() == 10);
    check(a.back() == 30);
};

auto arrayData = test("Array.data_points_to_first_element") = []
{
    auto a = EA::Array<int, 3> {1, 2, 3};
    check(a.data()[0] == 1);
    check(a.data()[2] == 3);
};

auto arrayLastElementIndex = test("Array.getLastElementIndex") = []
{
    auto a = EA::Array<int, 5>();
    check(a.getLastElementIndex() == 4);
};

auto arrayEqualityEqual = test("Array.operator==_equal") = []
{
    auto a = EA::Array<int, 3> {1, 2, 3};
    auto b = EA::Array<int, 3> {1, 2, 3};
    check(a == b);
    check(!(a != b));
};

auto arrayEqualityDifferent = test("Array.operator==_different") = []
{
    auto a = EA::Array<int, 3> {1, 2, 3};
    auto b = EA::Array<int, 3> {1, 2, 4};
    check(!(a == b));
    check(a != b);
};

auto arrayEqualityConst = test("Array.operator==_works_on_const") = []
{
    const auto a = EA::Array<int, 3> {1, 2, 3};
    const auto b = EA::Array<int, 3> {1, 2, 3};
    check(a == b);
    check(!(a != b));
};

auto arrayDeducedSize = test("Array.deduced_size_without_explicit_size") = []
{
    EA::Array a {1, 2, 3, 4};
    static_assert(std::is_same_v<decltype(a), EA::Array<int, 4>>);
    static_assert(decltype(a)::size() == 4);
    check(a.size() == 4);
    check(a[0] == 1);
    check(a[1] == 2);
    check(a[2] == 3);
    check(a[3] == 4);
};

auto arrayDeducedElementType = test("Array.deduced_size_deduces_element_type") = []
{
    EA::Array a {1.5, 2.5, 3.5};
    static_assert(std::is_same_v<decltype(a), EA::Array<double, 3>>);
    static_assert(std::is_same_v<decltype(a)::value_type, double>);
    check(a.size() == 3);
    check(a[2] == 3.5);
};

auto arrayDeducedSingleElement = test("Array.deduced_size_single_element") = []
{
    EA::Array a {42};
    static_assert(std::is_same_v<decltype(a), EA::Array<int, 1>>);
    check(a.size() == 1);
    check(a[0] == 42);
};

auto arrayPredicateHelpers = test("Array.predicate_helper_members") = []
{
    auto values = EA::Array<int, 4> {1, 2, 3, 4};

    check(values.getIndexIf([](int value) { return value > 2; }) == 2);
    check(values.countIf([](int value) { return value % 2 == 0; }) == 2);
    check(*values.findIf([](int value) { return value == 3; }) == 3);
    check(values.findIf([](int value) { return value == 99; }) == nullptr);

    *values.findIf([](int value) { return value == 3; }) = 30;
    check(values[2] == 30);
};

//Everything below runs entirely at compile time. The static_asserts are the
//test - a failure is a build error rather than a red case - and check() is
//there only so the case registers and reports.
auto arrayConstexprConstruction = test("Array.constexpr_construction") = []
{
    constexpr auto zeroed = EA::Array<int, 3> {};
    static_assert(zeroed[0] == 0 && zeroed[1] == 0 && zeroed[2] == 0);
    static_assert(zeroed.size() == 3);
    static_assert(!zeroed.empty());

    constexpr auto listed = EA::Array<int, 4> {1, 2, 3, 4};
    static_assert(listed[0] == 1 && listed[3] == 4);
    static_assert(listed.front() == 1);
    static_assert(listed.back() == 4);
    static_assert(listed.get(2) == 3);
    static_assert(listed.getLastElementIndex() == 3);

    //A shorter list leaves the rest value-initialized, at compile time too
    constexpr auto partial = EA::Array<int, 4> {1, 2};
    static_assert(partial[2] == 0 && partial[3] == 0);

    constexpr auto deduced = EA::Array {1.5, 2.5};
    static_assert(std::is_same_v<decltype(deduced), const EA::Array<double, 2>>);
    static_assert(deduced[1] == 2.5);

    check(true);
};

auto arrayConstexprComparison = test("Array.constexpr_comparison") = []
{
    constexpr auto a = EA::Array<int, 3> {1, 2, 3};
    constexpr auto b = EA::Array<int, 3> {1, 2, 3};
    constexpr auto c = EA::Array<int, 3> {1, 2, 4};

    static_assert(a == b);
    static_assert(a != c);

    check(true);
};

//The mutating members are constexpr too, which is what makes an Array usable as
//a local inside a constexpr function rather than only as a finished constant.
auto arrayConstexprMutation = test("Array.constexpr_mutation") = []
{
    constexpr auto filled = []
    {
        auto values = EA::Array<int, 4> {};
        values.fill(7);
        values[1] = 9;

        return values;
    }();

    static_assert(filled[0] == 7 && filled[1] == 9 && filled[3] == 7);

    constexpr auto mixed = []
    {
        auto values = EA::Array<int, 3> {1, 2, 3};
        auto other = EA::Array<int, 3> {10, 20, 30};
        values.mixFrom(other);

        return values;
    }();

    static_assert(mixed[0] == 11 && mixed[2] == 33);

    constexpr auto copied = []
    {
        auto source = EA::Array<int, 3> {4, 5, 6};
        auto target = EA::Array<int, 3> {};
        target.copyFrom(source);

        return target;
    }();

    static_assert(copied[1] == 5);

    check(true);
};

auto arrayConstexprSearching = test("Array.constexpr_searching") = []
{
    constexpr auto values = EA::Array<int, 4> {1, 2, 3, 4};

    static_assert(values.contains(3));
    static_assert(!values.contains(99));
    static_assert(values.getIndexOf(2) == 1);
    static_assert(values.getIndexOf(99) == -1);
    static_assert(values.getIndexIf([](int value) { return value > 2; }) == 2);
    static_assert(values.countIf([](int value) { return value % 2 == 0; }) == 2);

    static_assert(
        []
        {
            auto local = EA::Array<int, 4> {1, 2, 3, 4};
            auto* found = local.findIf([](int v) { return v == 3; });

            return found != nullptr && *found == 3;
        }());

    static_assert(values.findIf([](int v) { return v == 99; }) == nullptr);

    check(true);
};

auto arrayConstexprIteration = test("Array.constexpr_iteration") = []
{
    constexpr auto total = []
    {
        auto values = EA::Array<int, 4> {1, 2, 3, 4};
        auto sum = 0;

        for (auto value: values)
            sum += value;

        return sum;
    }();

    static_assert(total == 10);

    static_assert(
        []
        {
            auto values = EA::Array<int, 3> {5, 6, 7};

            return *values.data() == 5 && *(values.cbegin() + 2) == 7;
        }());

    check(true);
};
