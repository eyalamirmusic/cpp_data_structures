#pragma once

#include <cstddef>
#include <utility>

namespace EA
{
//Uninitialized in-place storage sized and aligned for a T: exactly sizeof(T)
//bytes (a union, so `memory` and `object` share storage). Use create() to
//placement-new a T into it and destroy() to run the destructor. The caller
//is responsible for tracking whether a T currently lives here — see
//Constructed<T> for a wrapper that manages that lifetime.
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
} // namespace EA
