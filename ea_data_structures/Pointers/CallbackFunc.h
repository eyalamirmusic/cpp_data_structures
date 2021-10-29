#pragma once

#include <functional>

//A little text-saver wrapper around simple single-args std::functions.
//Adds a default initialization with an empty function when needed

namespace EA
{
template <typename ArgType, typename ReturnType = void>
struct Callback
{
    Callback() = default;

    template <typename Callable>
    Callback(Callable funcToUse) noexcept
        : func(std::move(funcToUse))
    {
    }

    ReturnType operator()(ArgType arg) const { return func(arg); }
    ReturnType call(ArgType arg) const { return func(arg); }

    template <typename Callable>
    Callback& operator=(const Callable& funcToUse)
    {
        func = funcToUse;
        return *this;
    }

    std::function<ReturnType(ArgType)> func = [](ArgType) { return ReturnType(); };
};

//A little text-saver wrapper around the so commonly used std::function<void>
//The main advantages are a shorter name + a default initialized version
//To avoid the need to always pass on an empty function.

struct CallbackFunc
{
    CallbackFunc() = default;

    template <typename Callable>
    CallbackFunc(Callable funcToUse) noexcept
        : func(std::move(funcToUse))
    {
    }

    void operator()() const { func(); }
    void call() const { func(); }

    template <typename Callable>
    CallbackFunc& operator=(const Callable& funcToUse)
    {
        func = funcToUse;
        return *this;
    }

    std::function<void()> func = [] {};
};

//Creates a chain of two callabales and merges them into one, used mostly for things like threading
//Where you might want to chain the function that creates the thread with the actual
//callback and then pass it on to another class that triggers it later.

template <typename CallableFirst, typename CallableSecond>
CallbackFunc chainFunctions(CallableFirst&& first, CallableSecond&& second)
{
    return [first, second]
    {
        first();
        second();
    };
}

} // namespace EA