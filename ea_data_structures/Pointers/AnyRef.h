#pragma once

#include <any>

namespace EA
{
struct AnyRef
{
    AnyRef() = default;

    template <typename T>
    AnyRef(T& object)
        : any(&object)
    {
    }

    template <typename T>
    T& get() const
    {
        return *std::any_cast<T*>(any);
    }

    std::any any;
};
}