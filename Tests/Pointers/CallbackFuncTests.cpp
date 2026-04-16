#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Pointers/CallbackFunc.h>
#include <string>

using namespace nano;

auto callbackFuncDefault = test("CallbackFunc.default_is_noop") = []
{
    auto cb = EA::CallbackFunc();
    cb();
    check(true);
};

auto callbackFuncInvokes = test("CallbackFunc.invokes_stored_lambda") = []
{
    auto called = false;
    auto cb = EA::CallbackFunc([&] { called = true; });
    cb();
    check(called);
};

auto callbackFuncReassign = test("CallbackFunc.reassignment_replaces") = []
{
    auto cb = EA::CallbackFunc();
    auto counter = 0;
    cb = [&] { ++counter; };
    cb();
    cb();
    check(counter == 2);
};

auto callbackFuncCallMethod = test("CallbackFunc.call_method_works") = []
{
    auto called = false;
    auto cb = EA::CallbackFunc([&] { called = true; });
    cb.call();
    check(called);
};

auto callbackWithArg = test("Callback.returns_value_with_arg") = []
{
    auto cb = EA::Callback<int, int>([](int x) { return x * 2; });
    check(cb(5) == 10);
};

auto callbackWithArgDefault = test("Callback.default_returns_default_value") = []
{
    auto cb = EA::Callback<int, int>();
    check(cb(42) == 0);
};

auto chainFunctionsTest = test("chainFunctions.invokes_in_order") = []
{
    auto order = std::string();
    auto chained = EA::chainFunctions([&] { order += "a"; },
                                      [&] { order += "b"; });
    chained();
    check(order == "ab");
};
