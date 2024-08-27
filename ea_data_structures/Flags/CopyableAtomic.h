#pragma once

#include "AtomicHelpers.h"

namespace EA
{
struct AtomicBase
{
};

template <typename T>
constexpr bool isAtomic()
{
    return std::is_base_of_v<AtomicBase, T>;
}

//A simple wrapper around std::atomic, making sure to always be lock free
template <typename T>
struct Atomic
    : std::atomic<T>
    , AtomicBase
{
    using std::atomic<T>::atomic;

    using std::atomic<T>::operator=;

    static Atomics::Validator<T> validator;
};

//A copyable version of std::atomic/EA::Atomic
template <class T>
struct CopyableAtomic : Atomic<T>
{
    CopyableAtomic() = default;

    CopyableAtomic(const T& other) { this->store(other); }

    CopyableAtomic(const CopyableAtomic& other)
        : CopyableAtomic(other.load())
    {
    }

    CopyableAtomic& operator=(const CopyableAtomic<T>& other)
    {
        this->store(other.load());
        return *this;
    }

    CopyableAtomic& operator=(const T& val)
    {
        this->store(val);
        return *this;
    }

    operator T() const { return this->load(); }
};

//Another simple wrapper, mostly to create template specializations and/or
//stop on breakpoints for debugging
template <typename T>
struct AtomicWrapper : AtomicBase
{
    AtomicWrapper(const T& initialValue = T())
        : value(initialValue)
    {
    }

    void store(const T& other) noexcept { value.store(other); }
    T load() const { return value.load(); }

    Atomic<T> value;
};

struct AtomicFlag
{
    int load() const noexcept { return flag.load(); }
    void update() noexcept { ++flag; }

    Atomic<int> flag {0};
};
} // namespace EA
