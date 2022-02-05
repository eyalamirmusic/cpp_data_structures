#pragma once

#include "../Pointers/OwningPointer.h"

namespace EA
{
//A stripped down singleton pattern, allow you generate a static object
//With defined creation time from anywhere
template <typename T, typename... Args>
T& getStaticObject(Args&&... args)
{
    static OwningPointer<T> object;

    if (object == nullptr)
        object.create(std::forward<Args>(args)...);

    return *object;
}

//Similar to getStaticObject, but using stack allocation
//For situation where it's important.
template <typename T, typename... Args>
T& getStaticStackObject(Args&&... args)
{
    static T object(std::forward<Args>(args)...);
    return object;
}

//Sometimes you just want the static object to call it's constructor for side
//effects. This is a more explicit way to do that
template <typename T, typename... Args>
void initStaticObject(Args&&... args)
{
    getStaticObject<T>(std::forward<Args>(args)...);
}

struct UpdateMarker
{
    void update() { ++lastUpdate; }

    bool operator==(const UpdateMarker& other) const
    {
        return lastUpdate == other.lastUpdate;
    }

    int lastUpdate = 0;
};

//A base class for CRTP patterns
template <typename T>
struct CRTP
{
    T* getConcrete() { return static_cast<T*>(this); }
};

} // namespace EA