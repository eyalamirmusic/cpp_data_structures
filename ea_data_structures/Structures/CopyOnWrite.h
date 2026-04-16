#pragma once

#include <memory>

namespace EA
{

//A value that copies cheaply (via shared_ptr) until someone assigns a new
//value. Readers share the same underlying object; writing `cow = newValue`
//allocates fresh storage instead of mutating in place, so other readers are
//unaffected. Use for immutable snapshots passed between threads or scopes.
template <typename T>
class CopyOnWrite
{
    using Storage = std::shared_ptr<const T>;

public:
    CopyOnWrite(const CopyOnWrite& other) = default;
    CopyOnWrite(CopyOnWrite&& other) noexcept = default;
    CopyOnWrite(CopyOnWrite& other) { object = other.object; }

    template <typename... Args>
    CopyOnWrite(Args&&... args)
    {
        object = std::make_shared<const T>(std::forward<Args>(args)...);
    }

    CopyOnWrite& operator=(const CopyOnWrite& other) = default;
    CopyOnWrite& operator=(CopyOnWrite& other)
    {
        object = other.object;
        return *this;
    }

    CopyOnWrite& operator=(CopyOnWrite&& other) noexcept = default;

    CopyOnWrite& operator=(const T& other)
    {
        object = std::make_shared<const T>(other);
        return *this;
    }

    const T* get() const { return object.get(); }
    const T* operator->() const { return get(); }

    const T& operator*() { return *object; }

private:
    Storage object;
};
} // namespace EA