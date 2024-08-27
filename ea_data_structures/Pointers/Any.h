#pragma once

#include "OwningPointer.h"

namespace EA
{
namespace AnyDetail
{
struct Base
{
    virtual ~Base() = default;
};

template <typename T>
struct Object
    : Base
    , T
{
    template <typename... Args>
    Object(Args&&... args)
        : T(std::forward<Args>(args)...)
    {
    }
};

} // namespace AnyDetail
struct Any
{
    template <typename T, typename... Args>
    T* create(Args&&... args)
    {
        return object.create<AnyDetail::Object<T>>(std::forward<Args>(args)...);
    }

    template <typename T>
    T* get()
    {
        return object.getAs<T>();
    }

    OwningPointer<AnyDetail::Base> object;
};
} // namespace EA