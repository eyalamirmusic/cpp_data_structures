#pragma once

#include "../Structures/Array.h"

namespace EA
{
template <typename T>
class Constructed
{
public:
    ~Constructed() { destroy(); }

    template <typename... Args>
    void create(Args&&... args)
    {
        destroy();
        object = new (memory) T(std::forward<Args>(args)...);
    }

    void destroy()
    {
        if (object != nullptr)
        {
            object->~T();
            object = nullptr;
        }
    }

    bool isValid() const { return object != nullptr; }

    bool operator==(T* other) const { return object != other; }
    bool operator==(T& other) const
    {
        if (isValid())
            return *object == other;

        return false;
    }

    Constructed& operator=(const Constructed& other)
    {
        if (!other.isValid())
            destroy();
        else
        {
            if (isValid())
                *object = *other.object;
            else
            {
                //"Cannot copy from an invalid object"
                assert(false);
            }
        }

        return *this;
    }

    operator T&() const { return &object; }
    T* operator->() const { return object; }
    T& operator*() const { return *object; }

    T* get() const { return object; }

private:
    alignas(T) std::byte memory[sizeof(T)];
    T* object = nullptr;
};

template <typename T, int numItems>
using ConstructedArray = Array<Constructed<T>, numItems>;
} // namespace EA
