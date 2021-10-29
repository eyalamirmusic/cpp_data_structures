#pragma once

#include <type_traits>

namespace EA
{
template <typename T>
class FixedDynamicArray
{
public:
    using Aligned = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

    template <typename... ARGS>
    explicit FixedDynamicArray(int sizeToUse, ARGS&&... args)
        : internalSize(sizeToUse)
    {
        if (internalSize > 0)
        {
            internalData = (T*) new Aligned[(size_t) internalSize]();

            for (auto& element: *this)
                new (&element) T(std::forward<ARGS>(args)...);
        }
    }

    ~FixedDynamicArray()
    {
        for (auto& element: *this)
            element.~T();

        delete[](Aligned*) data();
    }

    T* begin() { return internalData; }
    T* end() { return internalData + internalSize; }

    T* begin() const { return internalData; }
    T* end() const { return internalData + internalSize; }

    T& operator[](int index) const { return internalData[(size_t) index]; }

    T* data() { return internalData; }
    int size() const { return internalSize; }

private:
    int internalSize = 0;
    T* internalData = nullptr;
};
} // namespace EA