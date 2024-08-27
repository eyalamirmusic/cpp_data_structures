#pragma once

#include <memory_resource>
#include <vector>

namespace EA::PMR
{
using Resource = std::pmr::memory_resource;

template <typename T>
struct Object
{
    using Type = T;

    template <typename... Args>
    Object(Resource* resourceToUse, Args&&... args)
        : resource(resourceToUse)
        , object(resource, std::forward<Args>(args)...)
    {
    }

    template <typename... Args>
    Object(Resource& resourceToUse, Args&&... args)
        : Object(&resourceToUse, std::forward<Args>(args)...)
    {
    }

    Object(const Object& other)
        : resource(other.resource)
        , object(other.resource)
    {
        object = other.object;
    }

    Object& operator=(const Object& other) = default;
    Object& operator=(Object&& other) noexcept = default;

    T& get() { return object; }
    const T& get() const { return object; }

    T* operator->() { return &object; }
    const T* operator->() const { return &object; }

    T& operator*() { return get(); }
    const T& operator*() const { return get(); }

    Resource* getResource() const { return resource; }

private:
    Resource* resource;
    T object;
};

template <typename T>
using Vector = Object<std::pmr::vector<T>>;

template <typename T, typename... Args>
std::shared_ptr<T> makeShared(Resource& resource, Args&&... args)
{
    auto allocator = std::pmr::polymorphic_allocator<T>(&resource);
    return std::allocate_shared<T>(allocator, std::forward<Args>(args)...);
}

} // namespace EA::PMR
