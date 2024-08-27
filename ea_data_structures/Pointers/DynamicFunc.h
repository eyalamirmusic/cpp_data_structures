#pragma once

#include "../Structures/Vector.h"
#include "AnyRef.h"
#include <functional>

namespace EA::DynamicFuncs
{
using Params = std::vector<AnyRef>;

struct Callable
{
    virtual ~Callable() = default;
    virtual void call(const Params& args) = 0;
};

template <typename... Args>
class ConcreteCallable final : public Callable
{
    using Func = std::function<void(Args...)>;

public:
    ConcreteCallable(const Func& funcToUse)
        : func(funcToUse)
    {
    }

    void call(const Params& args) override
    {
        call_impl(args, std::index_sequence_for<Args...> {});
    }

private:
    template <std::size_t... I>
    void call_impl(const Params& args, std::index_sequence<I...>)
    {
        func(args[I]
                 .template get<
                     std::remove_const_t<std::remove_reference_t<Args>>>()...);
    }

    Func func;
};

// Helper to extract function argument types
template <typename T>
struct function_traits;

template <typename R, typename... Args>
struct function_traits<R (*)(Args...)>
{
    using type = ConcreteCallable<Args...>;
};

// Create a Callable from any function pointer
template <typename Func>
auto create(Func func)
{
    using callable_type = typename function_traits<Func>::type;
    return std::make_unique<callable_type>(std::function(func));
}

} // namespace EA::DynamicFuncs
