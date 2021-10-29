#pragma once

#include "AtomicHelpers.h"

namespace EA
{
//A simple wrapper around std::atomic, making sure to always be lock free
template <typename T>
struct Atomic : std::atomic<T>
{
    using std::atomic<T>::atomic;

    Atomics::Validator<T> validator;
};

//A copyable version of std::atomic/EA::Atomic
template <class T>
struct CopyableAtomic: Atomic<T>
{
    using Atomic<T>::Atomic;

    CopyableAtomic(const CopyableAtomic<T>& other)
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
} // namespace EA
