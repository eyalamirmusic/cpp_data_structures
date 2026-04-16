#pragma once

#include <any>

namespace EA
{
//A type-erased non-owning reference to an object of any type. Stores the
//object's address inside a std::any; retrieve with get<T>() where T matches
//the original type. Used by DynamicFunc to pass arbitrary runtime arguments.
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