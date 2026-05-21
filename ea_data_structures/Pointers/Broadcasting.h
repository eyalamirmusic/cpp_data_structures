#pragma once

#include "Broadcaster.h"
#include <concepts>
#include <utility>

//Value wrappers that behave just like a T but trigger a Broadcaster every time
//the underlying value actually changes:
//
//    Broadcasting<int> count;       // owns the int
//    int storage = 0;
//    BroadcastingRef<int> view {storage}; // references an external int
//
//    Listener l {count, [&] { /* react */ }, EA::Listener::Modes::TriggerOnEvent};
//    count = 7;     // fires the listener
//    count = 7;     // no-op, value is unchanged
//    count += 3;    // fires once with the new value
//
//Both wrappers inherit from BroadcasterOwner, so any code that already accepts
//a Broadcaster or a "thing with getBroadcaster()" works with them unchanged.
//
//Triggering is gated on inequality whenever T is equality-comparable; for
//other types every mutating operation triggers. Reads go through an implicit
//conversion to const T& (and operator->), so the wrappers stay drop-in for
//const uses of T.

namespace EA
{

template <typename T>
class Broadcasting : public BroadcasterOwner
{
public:
    Broadcasting() = default;

    Broadcasting(T initial)
        : value(std::move(initial))
    {
    }

    Broadcasting(const Broadcasting&) = default;
    Broadcasting(Broadcasting&&) noexcept = default;

    operator const T&() const { return value; }
    const T& get() const { return value; }
    const T* operator->() const { return &value; }

    Broadcasting& operator=(const Broadcasting& other)
    {
        setValue(other.value);
        return *this;
    }

    Broadcasting& operator=(Broadcasting&& other) noexcept
    {
        setValue(std::move(other.value));
        return *this;
    }

    Broadcasting& operator=(const T& newValue)
    {
        setValue(newValue);
        return *this;
    }

    Broadcasting& operator=(T&& newValue)
    {
        setValue(std::move(newValue));
        return *this;
    }

    template <typename U>
    Broadcasting& operator+=(const U& rhs)
    {
        setValue(value + rhs);
        return *this;
    }

    template <typename U>
    Broadcasting& operator-=(const U& rhs)
    {
        setValue(value - rhs);
        return *this;
    }

    template <typename U>
    Broadcasting& operator*=(const U& rhs)
    {
        setValue(value * rhs);
        return *this;
    }

    template <typename U>
    Broadcasting& operator/=(const U& rhs)
    {
        setValue(value / rhs);
        return *this;
    }

    Broadcasting& operator++()
    {
        setValue(value + T(1));
        return *this;
    }

    Broadcasting& operator--()
    {
        setValue(value - T(1));
        return *this;
    }

private:
    template <typename U>
    void setValue(U&& newValue)
    {
        if constexpr (std::equality_comparable<T>)
        {
            if (value == newValue)
                return;
        }

        value = std::forward<U>(newValue);
        trigger();
    }

    T value {};
};

template <typename T>
class BroadcastingRef : public BroadcasterOwner
{
public:
    explicit BroadcastingRef(T& target)
        : value(target)
    {
    }

    BroadcastingRef(const BroadcastingRef&) = delete;
    BroadcastingRef(BroadcastingRef&&) = delete;

    operator const T&() const { return value; }
    const T& get() const { return value; }
    const T* operator->() const { return &value; }

    BroadcastingRef& operator=(const BroadcastingRef& other)
    {
        if (this != &other)
            setValue(other.value);
        return *this;
    }

    BroadcastingRef& operator=(const T& newValue)
    {
        setValue(newValue);
        return *this;
    }

    BroadcastingRef& operator=(T&& newValue)
    {
        setValue(std::move(newValue));
        return *this;
    }

    template <typename U>
    BroadcastingRef& operator+=(const U& rhs)
    {
        setValue(value + rhs);
        return *this;
    }

    template <typename U>
    BroadcastingRef& operator-=(const U& rhs)
    {
        setValue(value - rhs);
        return *this;
    }

    template <typename U>
    BroadcastingRef& operator*=(const U& rhs)
    {
        setValue(value * rhs);
        return *this;
    }

    template <typename U>
    BroadcastingRef& operator/=(const U& rhs)
    {
        setValue(value / rhs);
        return *this;
    }

    BroadcastingRef& operator++()
    {
        setValue(value + T(1));
        return *this;
    }

    BroadcastingRef& operator--()
    {
        setValue(value - T(1));
        return *this;
    }

private:
    template <typename U>
    void setValue(U&& newValue)
    {
        if constexpr (std::equality_comparable<T>)
        {
            if (value == newValue)
                return;
        }

        value = std::forward<U>(newValue);
        trigger();
    }

    T& value;
};

} // namespace EA
