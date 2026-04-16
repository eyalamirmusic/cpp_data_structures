#pragma once

#include <cstddef>
#include <utility>

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
} // namespace EA
