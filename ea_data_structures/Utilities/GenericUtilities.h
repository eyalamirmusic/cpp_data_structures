#pragma once

namespace EA
{
template <typename T, typename... Args>
constexpr bool equalsToAny(const T& a, Args&&... args) noexcept
{
    return ((a == args) || ...);
}

template <typename T, typename... Args>
constexpr bool equalsToAll(const T& a, Args&&... args) noexcept
{
    return ((a == args) && ...);
}

inline void toggleBool(bool& value) noexcept
{
    value = !value;
}
} // namespace EA