#pragma once

#include <tuple>

namespace EA::Tuples
{
template <typename Tuple, typename FuncType, std::size_t... I>
constexpr FuncType forEachImpl(Tuple&& t, FuncType&& f, std::index_sequence<I...>)
{
    return (void) std::initializer_list<int> {
               (std::forward<FuncType>(f)(std::get<I>(std::forward<Tuple>(t))),
                0)...},
           f;
}

template <typename Tuple, typename FuncType>
constexpr FuncType for_each(Tuple&& t, FuncType&& f)
{
    return forEachImpl(
        std::forward<Tuple>(t),
        std::forward<FuncType>(f),
        std::make_index_sequence<
            std::tuple_size_v<std::remove_reference_t<Tuple>>> {});
}

template <typename Container, typename FuncType>
constexpr void forEach(Container& container, FuncType&& f)
{
    for_each(container.getMembers(), std::forward<FuncType>(f));
}

template <typename... Ts>
auto make_pointer_tuple()
{
    return std::tuple {static_cast<Ts*>(nullptr)...};
}

template <typename... Args, typename FuncType>
void callForAllTypes(FuncType&& func)
{
    (func(static_cast<Args*>(nullptr)), ...);
}

template <typename... Args, typename T, typename FuncType>
void callIfTypeMatching(T& obj, FuncType&& callback)
{
    auto func = [&](auto element)
    {
        if (auto* cast = dynamic_cast<decltype(element)>(&obj))
            callback(*cast);
    };

    callForAllTypes<Args...>(func);
}

template <typename... Args>
struct Container
{
    template <typename FuncType>
    constexpr FuncType forEach(FuncType&& f)
    {
        return for_each(elements, std::forward<FuncType>(f));
    }

    template <typename T>
    constexpr auto& get()
    {
        return std::get<T>(elements);
    }

    template <int index>
    constexpr auto& get()
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
