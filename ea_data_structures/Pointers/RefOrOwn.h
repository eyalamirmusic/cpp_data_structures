#pragma once

#include <memory>

namespace EA
{
template <typename T>
struct RefOrOwn
{
    using Owned = std::shared_ptr<T>;

    RefOrOwn() = default;
    RefOrOwn(const RefOrOwn& other) { set(other); }
    RefOrOwn(RefOrOwn& other) { set(other); }

    RefOrOwn(T& objToUse) { set(&objToUse); }
    RefOrOwn(T* objToUse) { set(objToUse); }

    RefOrOwn(const Owned& ownedToUse) { set(ownedToUse); }

    RefOrOwn& operator=(const RefOrOwn& other)
    {
        if (this != &other)
            set(other);

        return *this;
    }
    void set(const Owned& ownedToUse)
    {
        owned = ownedToUse;
        object = owned.get();
    }

    void set(const RefOrOwn& other)
    {
        if (other.object == other.owned.get())
            set(other.owned);
        else
            set(other.object);
    }

    void set(T* objectToUse)
    {
        owned = nullptr;
        object = objectToUse;
    }

    template <typename... Args>
    T& create(Args&&... args)
    {
        auto created = std::make_shared<T>(std::forward<Args>(args)...);
        set(created);

        return *get();
    }

    template <typename... Args>
    T& createIfNeeded(Args&&... args)
    {
        if (get() == nullptr)
            return create(std::forward<Args>(args)...);

        return *object;
    }

    T* get() { return object; }
    const T* get() const { return object; }

    T* operator->() { return object; }
    const T* operator->() const { return object; }

    T& operator*() { return *object; }
    const T& operator*() const { return *object; }

    T* object = nullptr;
    Owned owned;
};
} // namespace EA