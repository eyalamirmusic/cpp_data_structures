#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>
#include <limits>

using namespace nano;

auto valueDefault = test("ValueWrapper.default_construction") = []
{
    auto v = EA::ValueWrapper<int>();
    check(int(v) == 0);
};

auto valueFromValue = test("ValueWrapper.construct_from_value") = []
{
    auto v = EA::ValueWrapper<int>(42);
    check(int(v) == 42);
};

auto valueAssign = test("ValueWrapper.setValue") = []
{
    auto v = EA::ValueWrapper<int>();
    v.setValue(7);
    check(int(v) == 7);
};

auto valueCopy = test("ValueWrapper.copy_construction") = []
{
    auto a = EA::ValueWrapper<int>(3);
    auto b = a;
    check(int(b) == 3);
};

auto valueEquality = test("ValueWrapper.equality_and_inequality") = []
{
    auto a = EA::ValueWrapper<int>(3);
    auto b = EA::ValueWrapper<int>(3);
    auto c = EA::ValueWrapper<int>(4);
    check(a == b);
    check(a != c);
    check(a == 3);
    check(a != 4);
};

auto valueIncrement = test("ValueWrapper.prefix_increment") = []
{
    auto v = EA::ValueWrapper<int>(5);
    ++v;
    check(int(v) == 6);
};

auto valueDecrement = test("ValueWrapper.prefix_decrement") = []
{
    auto v = EA::ValueWrapper<int>(5);
    --v;
    check(int(v) == 4);
};

auto valuePlusEquals = test("ValueWrapper.plus_equals") = []
{
    auto v = EA::ValueWrapper<int>(10);
    v += 5;
    check(int(v) == 15);
};

auto valueTimesEquals = test("ValueWrapper.times_equals") = []
{
    auto v = EA::ValueWrapper<int>(4);
    v *= 3;
    check(int(v) == 12);
};

auto valueMinMax = test("ValueWrapper.min_and_max_limits") = []
{
    using W = EA::ValueWrapper<int>;
    check(W::getMin() == std::numeric_limits<int>::min());
    check(W::getMax() == std::numeric_limits<int>::max());
};

auto valueGetValueRef = test("ValueWrapper.getValue_returns_ref") = []
{
    auto v = EA::ValueWrapper<int>(5);
    auto& r = v.getValue();
    r = 99;
    check(int(v) == 99);
};
