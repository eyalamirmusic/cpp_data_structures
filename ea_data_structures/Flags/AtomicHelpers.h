#pragma once

#include <atomic>

namespace EA::Atomics
{
template <typename T>
constexpr bool isLockFree()
{
    return std::atomic<T>::is_always_lock_free;
}

template <typename T>
constexpr void validateLockFree()
{
    static_assert(isLockFree<T>(),
                  "Can only use safely with small lock free objects");
}

template <typename T>
struct Validator
{
    Validator() { validateLockFree<T>(); }
};
} // namespace EA::Atomics
