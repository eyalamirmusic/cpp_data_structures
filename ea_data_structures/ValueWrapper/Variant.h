#pragma once

#include "VariantVisit.h"

namespace EA
{
//A thin wrapper around std::variant with a slightly simpler syntax:
//get<T>() returns a pointer to the active alternative (or nullptr), and
//visit() dispatches through Rollbear::visit, which compiles to a chain of
//inlined index comparisons instead of std::visit's jump table.
//Everything else (constructors, index(), emplace(), comparisons, etc.) is
//inherited from std::variant.
template <typename... Args>
struct Variant : std::variant<Args...>
{
    using VarType = std::variant<Args...>;
    using VarType::variant;

    template <typename T>
    constexpr bool holds() const noexcept
    {
        return std::holds_alternative<T>(*this);
    }

    template <typename T>
    constexpr T* get() noexcept
    {
        return std::get_if<T>(this);
    }

    template <typename T>
    constexpr const T* get() const noexcept
    {
        return std::get_if<T>(this);
    }

    template <typename Callable>
    constexpr auto visit(Callable&& func)
    {
        return Rollbear::visit(func, *this);
    }

    template <typename Callable>
    constexpr auto visit(Callable&& func) const
    {
        return Rollbear::visit(func, *this);
    }
};
} // namespace EA
