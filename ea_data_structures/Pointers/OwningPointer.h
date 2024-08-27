#pragma once

#include <memory>

namespace EA
{
//Owning pointer: a lightweight class, similar to std::unique_ptr
//Made for slightly easier debugging, and some different semantics

//For example, unlike std::unqiue_ptr you can pass this object to any class
//requiring a T*, so it's an easy replace for raw pointers

//This class also supports polymorphic copy semantics (using clone() method in the derived classes

template <typename T>
class OwningPointer
{
public:
    OwningPointer() = default;

    template <typename A>
    OwningPointer(const OwningPointer<A>& other)
    {
        copy(other);
    }

    OwningPointer(const OwningPointer& other) { copy(other); }

    template <typename A>
    OwningPointer(OwningPointer<A>&& other) noexcept
    {
        reset(other.release());
    }

    OwningPointer(std::unique_ptr<T>&& other) noexcept { reset(other.release()); }
    OwningPointer(std::unique_ptr<T>& other) { reset(other.release()); }
    OwningPointer(T* other) { reset(other); }

    ~OwningPointer() { reset(); }

    OwningPointer& operator=(OwningPointer&& other) noexcept
    {
        reset(other.release());
        return *this;
    }

    template <typename A>
    OwningPointer& operator=(OwningPointer<A>&& other) noexcept
    {
        reset(other.release());
        return *this;
    }

    OwningPointer& operator=(std::unique_ptr<T>&& other) noexcept
    {
        reset(other.release());
        return *this;
    }

    OwningPointer& operator=(T* other)
    {
        reset(other);
        return *this;
    }

    template <typename A>
    OwningPointer& operator=(const OwningPointer<A>& other)
    {
        if (&other == this)
            return *this;

        copy(other);
        return *this;
    }

    OwningPointer& operator=(const OwningPointer& other)
    {
        if (&other == this)
            return *this;

        copy(other);
        return *this;
    }

    bool operator<(const OwningPointer& other) const
    {
        return *object < *other.object;
    }

    template <typename A>
    void copy(const OwningPointer<A>& other)
    {
        if (other != nullptr)
            reset(other->clone());
        else
            reset();
    }

    template <typename A = T>
    void reset(A* other = nullptr)
    {
        delete object;
        object = other;
    }

    template <typename... Args>
    T& getOrCreate(Args&&... args)
    {
        if (object == nullptr)
            create(std::forward<Args>(args)...);

        return *object;
    }

    template <typename... Args>
    T* create(Args&&... args)
    {
        reset(new T(std::forward<Args>(args)...));
        return object;
    }

    template <typename Derived, typename... Args>
    Derived* create(Args&&... args)
    {
        reset(new Derived(std::forward<Args>(args)...));
        return getAs<Derived>();
    }

    template <typename A>
    A* getAs() const
    {
        return dynamic_cast<A*>(get());
    }

    T* get() const { return object; }
    operator T*() const { return object; }
    T* operator->() const { return object; }
    T& operator*() const { return *object; }
    bool operator==(T* other) const { return object == other; }
    bool operator!=(T* other) const { return object != other; }

    T* release()
    {
        T* pointer = object;
        object = nullptr;
        return pointer;
    }

private:
    T* object = nullptr;
};

template <typename T, typename... Args>
OwningPointer<T> makeOwned(Args&&... args)
{
    OwningPointer<T> result;
    result.create(std::forward<Args>(args)...);
    return result;
}
} // namespace EA
