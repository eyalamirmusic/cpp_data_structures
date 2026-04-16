#pragma once

#include <limits>

namespace EA
{
//Wraps a scalar T with default-initialization to T{} and the usual arithmetic
//and comparison operators forwarded to the underlying value. Useful as a
//building block when you want a default-initialized member or want to give a
//scalar a distinct type via a `using Foo = ValueWrapper<int>;` alias.
template <typename T>
class ValueWrapper
{
public:
    ValueWrapper() = default;
    ValueWrapper(const T& newValue) { setValue(newValue); }

    ValueWrapper(const ValueWrapper& other) { setValue(other.value); }
    ValueWrapper(ValueWrapper&& newValue) noexcept = default;

    operator T() const { return value; }

    void setValue(const T& newValue) { value = newValue; }

    bool operator==(const ValueWrapper& other) const { return value == other.value; }
    bool operator!=(const ValueWrapper& other) const { return !operator==(other); }

    bool operator==(const T& other) const { return value == other; }
    bool operator!=(const T& other) const { return !operator==(other); }

    ValueWrapper& operator=(const ValueWrapper& newValue)
    {
        setValue(newValue.value);
        return *this;
    }

    static constexpr T getMin() { return std::numeric_limits<T>::min(); }

    static constexpr T getMax() { return std::numeric_limits<T>::max(); }

    ValueWrapper& operator=(const T& newValue)
    {
        setValue(newValue);
        return *this;
    }

    ValueWrapper& operator+=(const T& newValue)
    {
        setValue(value + newValue);
        return *this;
    }

    ValueWrapper& operator*=(const T& newValue)
    {
        setValue(value * newValue);
        return *this;
    }

    ValueWrapper& operator/=(T newValue)
    {
        setValue(value / newValue);
        return *this;
    }

    ValueWrapper& operator++()
    {
        setValue(value + T(1));
        return *this;
    }

    ValueWrapper& operator--()
    {
        setValue(value - T(1));
        return *this;
    }

    T get() { return value; }

    T& getValue() { return value; }

private:
    T value {};
};
} // namespace EA
