#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

//A modified version of https://github.com/rollbear/visit, ported from
//https://github.com/eyalamirmusic/Variant.
//
//Instead of std::visit's table of function pointers, visitation walks every
//combination of alternative indices at compile time and emits a chain of
//index comparisons that each call the visitor directly. The last combination
//is called unconditionally. Compilers inline the whole chain, which produces
//much better code than std::visit on some toolchains, and it also compiles on
//older macOS deployment targets where std::visit / std::get are unavailable.
//
//Arguments may be std::variants (or types derived from one, like EA::Variant)
//or plain values, which behave like a variant with a single alternative.
//
//Precondition: no visited variant is valueless_by_exception().
namespace EA::Rollbear
{
namespace Detail
{
template <typename... Ts>
std::variant<Ts...> variantAccessImpl(const std::variant<Ts...>*);

template <typename T>
using VariantAccess =
    decltype(variantAccessImpl(static_cast<std::decay_t<T>*>(nullptr)));

template <template <typename...> class, typename = void, typename...>
struct Detected : std::false_type
{
};

template <template <typename...> class D, typename... Ts>
struct Detected<D, std::void_t<D<Ts...>>, Ts...> : std::true_type
{
};

template <template <typename...> class D, typename... Ts>
using IsDetected = typename Detected<D, void, Ts...>::type;

template <template <typename...> class D, typename... Ts>
constexpr bool isDetected = IsDetected<D, Ts...>::value;

template <typename T>
constexpr bool isVariant = isDetected<VariantAccess, T>;

template <std::size_t I, std::size_t... Is>
constexpr std::index_sequence<I, Is...> prepend(std::index_sequence<Is...>)
{
    return {};
}

constexpr std::index_sequence<> nextSeq(std::index_sequence<>, std::index_sequence<>)
{
    return {};
}

template <typename T, typename V>
struct CopyReferencenessImpl
{
    using type = T;
};

template <typename T, typename V>
struct CopyReferencenessImpl<T, V&>
{
    using type = T&;
};

template <typename T, typename V>
struct CopyReferencenessImpl<T, V&&>
{
    using type = std::remove_reference_t<T>&&;
};

template <typename T, typename V>
using CopyReferenceness = typename CopyReferencenessImpl<T, V>::type;

template <typename T, typename TSource>
using AsIfForwarded =
    std::conditional_t<!std::is_reference<TSource> {},
                       std::add_rvalue_reference_t<std::remove_reference_t<T>>,
                       CopyReferenceness<T, TSource>>;

template <typename TLike, typename T>
constexpr decltype(auto) forwardLike(T&& x) noexcept
{
    static_assert(!(std::is_rvalue_reference<decltype(x)> {}
                    && std::is_lvalue_reference<TLike> {}));

    return static_cast<AsIfForwarded<T, TLike>>(x);
}

template <std::size_t I, std::size_t... Is, std::size_t J, std::size_t... Js>
constexpr auto nextSeq(std::index_sequence<I, Is...>, std::index_sequence<J, Js...>)
{
    if constexpr (I + 1 == J)
    {
        return prepend<0>(
            nextSeq(std::index_sequence<Is...> {}, std::index_sequence<Js...> {}));
    }
    else
    {
        return std::index_sequence<I + 1, Is...> {};
    }
}

template <std::size_t... I>
constexpr std::size_t sum(std::index_sequence<I...>)
{
    return (I + ...);
}

template <std::size_t I, typename T>
constexpr decltype(auto) get(T&& t)
{
    if constexpr (isVariant<T>)
    {
        return *std::get_if<I>(&std::forward<T>(t));
    }
    else
    {
        static_assert(I == 0);
        return std::forward<T>(t);
    }
}

template <std::size_t I, typename T>
constexpr auto getIf(T* t)
{
    if constexpr (isVariant<T>)
    {
        return std::get_if<I>(t);
    }
    else
    {
        static_assert(I == 0);
        return t;
    }
}

template <typename V>
constexpr std::size_t variantSize()
{
    if constexpr (isVariant<V>)
    {
        return std::variant_size_v<VariantAccess<V>>;
    }
    else
    {
        return 1;
    }
}

template <typename V>
constexpr std::size_t index(const V& v)
{
    if constexpr (isVariant<V>)
    {
        return v.index();
    }
    else
    {
        return 0;
    }
}

template <std::size_t... Is, std::size_t... Ms, typename F, typename... Vs>
inline constexpr auto visit(std::index_sequence<Is...> i,
                            std::index_sequence<Ms...> m,
                            F&& f,
                            Vs&&... vs)
{
    constexpr auto n = nextSeq(i, m);
    if constexpr (sum(n) == 0)
    {
        return f(get<Is>(std::forward<Vs>(vs))...);
    }
    else
    {
        if (std::tuple(Detail::index(vs)...) == std::tuple(Is...))
        {
            return f(forwardLike<Vs>(*getIf<Is>(&vs))...);
        }
        return visit(n, m, std::forward<F>(f), std::forward<Vs>(vs)...);
    }
}

template <typename>
inline constexpr std::size_t zero = 0;
} // namespace Detail

template <typename F, typename... Vs>
inline constexpr auto visit(F&& f, Vs&&... vs)
{
    if constexpr (((Detail::variantSize<Vs>() == 1) && ...))
    {
        return f(Detail::forwardLike<Vs>(*Detail::getIf<0>(&vs))...);
    }
    else
    {
        return Detail::visit(std::index_sequence<Detail::zero<Vs>...> {},
                             std::index_sequence<Detail::variantSize<Vs>()...> {},
                             std::forward<F>(f),
                             std::forward<Vs>(vs)...);
    }
}
} // namespace EA::Rollbear
