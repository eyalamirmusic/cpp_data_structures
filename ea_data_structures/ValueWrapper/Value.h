#pragma once

namespace EA
{
template <typename T>
class ValueWrapper
{
public:
    ValueWrapper() = default;
    explicit ValueWrapper(const T& newValue) { setValue(newValue); }

    ValueWrapper(const ValueWrapper& other) { setValue(other.value); }
    ValueWrapper(ValueWrapper&& newValue) noexcept = default;

    explicit operator T() { return value; }

    void setValue(const T& newValue) { value = newValue; }

    ValueWrapper& operator=(const ValueWrapper& newValue)
    {
        setValue(newValue.value);
        return *this;
    }

    ValueWrapper& operator=(T newValue) noexcept
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
        setValue(newValue / newValue);
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

private:
    T value {};
};
} // namespace EA
