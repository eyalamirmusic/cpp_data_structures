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

//A type-erased owner for objects of any class type. create<T>(args...) stores a
//T and returns a T*; get<T>() returns the stored pointer as T* (or nullptr if
//T is not the stored type). Object destruction runs through a virtual base.
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