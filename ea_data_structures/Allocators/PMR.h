#pragma once

#include <cstddef>
#include <memory>
#include <memory_resource>
#include <type_traits>
#include <utility>
#include <vector>

namespace EA::PMR
{
using Resource = std::pmr::memory_resource;

//Pairs a PMR-aware object T with the memory_resource it was constructed with.
//The resource pointer is non-owning: the caller must keep the resource alive
//for longer than the Object and every copy of it.
//Copies and moves are constructed on the source's resource. Assignment follows
//std::pmr semantics: the target keeps its own resource and only the contents
//are assigned, so getResource() always names the resource T allocates from.
//Typical T is a std::pmr container; see the `Vector` alias below.
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

    Object(Object&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : resource(other.resource)
        , object(std::move(other.object))
    {
    }

    Object& operator=(const Object& other)
    {
        if (this != &other)
            object = other.object;

        return *this;
    }

    Object& operator=(Object&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        object = std::move(other.object);
        return *this;
    }

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

//An Owned<From> may become an Owned<To> when only cv-qualifiers change, or when
//To is a base with a virtual destructor, so deleting through it is complete.
template <typename From, typename To>
concept OwnedConvertibleTo =
    std::is_convertible_v<From*, To*>
    && (std::is_same_v<std::remove_cv_t<From>, std::remove_cv_t<To>>
        || std::has_virtual_destructor_v<To>);

//Deleter for Owned<T>: destroys the object and hands its memory back to the
//resource it came from, with the size and alignment captured at creation.
//Converting to Owned<Base> keeps the original size and alignment, and is only
//allowed when Base has a virtual destructor, so the whole object is destroyed.
template <typename T>
struct Deleter
{
    Deleter() = default;

    Deleter(Resource* resourceToUse, size_t sizeToUse, size_t alignmentToUse)
        : resource(resourceToUse)
        , size(sizeToUse)
        , alignment(alignmentToUse)
    {
    }

    template <OwnedConvertibleTo<T> U>
    Deleter(const Deleter<U>& other) noexcept
        : resource(other.resource)
        , size(other.size)
        , alignment(other.alignment)
    {
    }

    void operator()(T* object) const
    {
        //The allocation starts at the most-derived object, which a Base*
        //into a multiply-inherited Derived doesn't necessarily point to.
        auto* mutableObject = const_cast<std::remove_cv_t<T>*>(object);
        void* memory = nullptr;

        if constexpr (std::is_polymorphic_v<T>)
            memory = dynamic_cast<void*>(mutableObject);
        else
            memory = static_cast<void*>(mutableObject);

        std::destroy_at(object);
        resource->deallocate(memory, size, alignment);
    }

    Resource* resource = nullptr;
    size_t size = 0;
    size_t alignment = 0;
};

//Uniquely owned T living in a memory_resource; see makeOwned().
template <typename T>
using Owned = std::unique_ptr<T, Deleter<T>>;

//The unique-ownership counterpart to makeShared(): allocates and constructs a
//T in `resource`, and the result destroys it and frees the memory back to the
//same resource. A PMR-aware T (e.g. a std::pmr container) is given the
//resource too, through uses-allocator construction. If T's constructor
//throws, the memory is returned to the resource before the exception leaves.
//The resource is not owned: it must outlive the result.
template <typename T, typename... Args>
Owned<T> makeOwned(Resource& resource, Args&&... args)
{
    auto allocator = std::pmr::polymorphic_allocator<>(&resource);
    auto* object = allocator.new_object<T>(std::forward<Args>(args)...);

    return Owned<T>(object, Deleter<T>(&resource, sizeof(T), alignof(T)));
}

} // namespace EA::PMR
