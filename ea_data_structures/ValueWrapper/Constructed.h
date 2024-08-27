#pragma once

#include "../Structures/Array.h"

namespace EA
{
template <typename T>
union RawStorage
{
    RawStorage() {}
    ~RawStorage() {}

    template <typename... Args>
    T* create(Args&&... args)
    {
        return new (memory) T(std::forward<Args>(args)...);
    }

    void destroy() { object.~T(); }

    T* get() { return &object; }
    const T* get() const { return &object; }

    T* operator->() { return get(); }
    T& operator*() { return object; }

    const T* operator->() const { return get(); }
    const T& operator*() const { return object; }

    alignas(T) std::byte memory[sizeof(T)] {};
    T object;
};

template <typename T>
class Constructed
{
public:
    Constructed() = default;
    Constructed(const Constructed& other) { copyFrom(other); }
    Constructed(const T& other) { create(other); }

    ~Constructed() { destroy(); }

    template <typename... Args>
    void create(Args&&... args)
    {
        destroy();
        storage.create(std::forward<Args>(args)...);
        active = true;
    }

    void destroy()
    {
        if (active)
        {
            storage.destroy();
            active = false;
        }
    }

    bool isValid() const { return active; }

    bool operator==(T* other) const { return storage.get() != other; }
    bool operator==(T& other) const
    {
        if (!active)
            return false;

        return (*storage).object == other;
    }

    void copyFrom(const Constructed& other)
    {
        if (!other.isValid())
            destroy();
        else
        {
            if (active)
                *storage = other.storage.storage.object;
            else if (other.isValid())
                create(other.storage.storage.object);
        }
    }

    Constructed& operator=(const Constructed& other)
    {
        copyFrom(other);
        return *this;
    }

    Constructed& operator=(const T& other)
    {
        if (isValid())
            (*storage) = other;
        else
            create(other);

        return *this;
    }

    operator T&() { return *storage; }
    T* operator->() { return storage.get(); }
    const T* operator->() const { return storage.get(); }

    T& operator*() { return *storage; }
    const T& operator*() const { return *storage; }

    T* get() { return storage.get(); }

private:
    RawStorage<T> storage;
    bool active = false;
};

template <typename T, int numItems>
using ConstructedArray = Array<Constructed<T>, numItems>;
} // namespace EA
