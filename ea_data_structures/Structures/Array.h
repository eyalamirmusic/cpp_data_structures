#pragma once

//A wrapper around std::array that uses int instead of size_t, and adds some useful helper functions

#include "../Utilities/VectorUtilities.h"
#include <array>
#include <concepts>

namespace EA
{
//A std::array wrapper with int-based sizes/indexes and helpers (contains,
//fill, sort, getIndexOf, …). Size is fixed at compile time via the Size
//template parameter; storage is in-place, no heap allocation.
template <typename T, int Size>
class Array
{
public:
    using value_type = T;
    using size_type = int;
    using ContainerType = std::array<T, (size_t) Size>;
    using Iterator = typename ContainerType::iterator;
    using Const_Iterator = typename ContainerType::const_iterator;

    Array() = default;

    Array(std::initializer_list<T> list)
    {
        auto numToCopy = (int) list.size() < Size ? (int) list.size() : Size;
        auto out = container.begin();

        for (auto it = list.begin(); it != list.begin() + numToCopy; ++it)
            *out++ = *it;
    }

    Array(const Array& other) = default;
    Array(Array&& other) noexcept = default;

    explicit Array(const ContainerType& other) { container = other; }

    explicit Array(ContainerType&& other) noexcept { container = std::move(other); }

    Array& operator=(const ContainerType& other)
    {
        container = other;
        return *this;
    }

    Array& operator=(const Array& other) = default;

    bool operator==(const Array& other) const
    {
        return container == other.container;
    }

    bool operator!=(const Array& other) const
    {
        return container != other.container;
    }

    bool empty() const noexcept { return container.empty(); }

    static constexpr int size() noexcept { return Size; }

    T& back() { return container.back(); }
    T& front() { return container.front(); }

    T& operator[](int index) noexcept { return container[(size_t) index]; }
    const T& operator[](int index) const noexcept
    {
        return container[(size_t) index];
    }
    T& get(int index) { return container[(size_t) index]; }
    const T& get(int index) const { return container[(size_t) index]; }

    Iterator begin() noexcept { return container.begin(); }
    Iterator end() noexcept { return container.end(); }

    Const_Iterator begin() const noexcept { return container.begin(); }
    Const_Iterator end() const noexcept { return container.end(); }

    Const_Iterator cbegin() const { return container.cbegin(); }
    Const_Iterator cend() const { return container.cend(); }

    bool contains(const T& element) const
    {
        return Vectors::contains(container, element);
    }

    ContainerType& getArray() { return container; }
    const ContainerType& getArray() const { return container; }
    void copyFrom(ContainerType& other) { container = other; }
    void copyFrom(Array& other) { container = other.getArray(); }

    template <typename A>
    void mixFrom(A& other)
    {
        for (int index = 0; index < size(); ++index)
            container[index] += other[index];
    }

    void fill(const T& value)
    {
        for (auto& element: container)
            element = value;
    }

    template <typename A>
    void fillFrom(A& other)
    {
        Vectors::copyInto(other, container);
    }

    int getLastElementIndex() const { return size() - 1; }

    void sort() { std::sort(begin(), end()); }

    template <typename A>
    int getIndexOf(const A& element) const
    {
        return Vectors::getIndexOf(container, element);
    }

    template <typename Predicate>
    int getIndexIf(Predicate&& predicate) const
    {
        return Vectors::getIndexIf(container, std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    int countIf(Predicate&& predicate) const
    {
        return Vectors::countIf(container, std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    const T* findIf(Predicate&& predicate) const
    {
        return Vectors::findIf(*this, std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    T* findIf(Predicate&& predicate)
    {
        return Vectors::findIf(*this, std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    void sort(const Predicate& pred, bool forward = true)
    {
        std::sort(begin(), end(), pred);

        if (!forward)
            Vectors::reverse(container);
    }

    const T* data() const { return container.data(); }
    T* data() { return container.data(); }

protected:
    ContainerType container {};
};

//Deduction guide: lets the size be deduced from the elements, so you can
//write EA::Array {1, 2, 3} instead of EA::Array<int, 3> {1, 2, 3}. All
//elements must share the same type (mirrors std::array's guide).
template <typename T, typename... U>
    requires(std::same_as<T, U> && ...)
Array(T, U...) -> Array<T, 1 + (int) sizeof...(U)>;
} // namespace EA
