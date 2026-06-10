#pragma once

#include <tuple>
#include <utility>

namespace EA::Tuples
{
template <typename Tuple, typename FuncType>
constexpr FuncType for_each(Tuple&& t, FuncType&& f)
{
    std::apply([&f](auto&&... elements)
               { (f(std::forward<decltype(elements)>(elements)), ...); },
               std::forward<Tuple>(t));

    return std::forward<FuncType>(f);
}

template <typename Container, typename FuncType>
    requires requires(Container& c) { c.getMembers(); }
constexpr void forEach(Container& container, FuncType&& f)
{
    for_each(container.getMembers(), std::forward<FuncType>(f));
}

template <typename... Ts>
[[nodiscard]] constexpr auto make_pointer_tuple()
{
    return std::tuple<Ts*...> {};
}

template <typename... Args, typename FuncType>
void callForAllTypes(FuncType&& func)
{
    (func(static_cast<Args*>(nullptr)), ...);
}

template <typename... Args, typename T, typename FuncType>
void callIfTypeMatching(T& obj, FuncType&& callback)
{
    auto func = [&]([[maybe_unused]] auto element)
    {
        if (auto* cast = dynamic_cast<decltype(element)>(&obj))
            callback(*cast);
    };

    callForAllTypes<Args...>(func);
}

//A tuple-like container that exposes forEach() plus get-by-type and
//get-by-index accessors. Intended for holding a heterogeneous set of
//members whose types are known at compile time.
template <typename... Args>
struct Container
{
    template <typename FuncType>
    constexpr FuncType forEach(FuncType&& f)
    {
        return for_each(elements, std::forward<FuncType>(f));
    }

    template <typename FuncType>
    constexpr FuncType forEach(FuncType&& f) const
    {
        return for_each(elements, std::forward<FuncType>(f));
    }

    template <typename T>
    constexpr auto& get()
    {
        return std::get<T>(elements);
    }

    template <typename T>
    constexpr auto& get() const
    {
        return std::get<T>(elements);
    }

    template <int index>
    constexpr auto& get()
    {
        return std::get<index>(elements);
    }

    template <int index>
    constexpr auto& get() const
    {
        return std::get<index>(elements);
    }

    std::tuple<Args...> elements;
};
} // namespace EA::Tuples

namespace MixedTypes
{
template <class F, class... Args>
void forEach(F&& f, Args&&... args)
{
    ((void) f(std::forward<Args>(args)), ...);
}
} // namespace MixedTypes
