#pragma once

#include "OwningPointer.h"

namespace EA
{
//A non-owning pointer wrapper. Constructible from a reference, a raw pointer,
//or an OwningPointer, and implicitly decays back to T& or T* at the point of
//use. Useful for passing objects whose ownership lives elsewhere.
template <typename T>
class Ref
{
public:
    Ref(T& objToUse)
        : object(&objToUse)
    {
    }

    Ref(T* objToUse)
        : object(objToUse)
    {
    }

    Ref(OwningPointer<T>& objToUse)
        : object(objToUse.get())
    {
    }

    Ref(const Ref& other) = default;
    Ref& operator=(const Ref& other) = default;

    T* operator->() { return object; }
    T* operator->() const { return object; }

    T& operator*() { return *object; }
    T& operator*() const { return *object; }

    T* get() { return object; }
    T* get() const { return object; }

    template <typename A>
    A* getAs()
    {
        return dynamic_cast<A*>(object);
    }

    operator T&() { return *object; }
    operator T&() const { return *object; }

    operator T*() { return object; }
    operator T*() const { return object; }

private:
    T* object = nullptr;
};
} // namespace EA