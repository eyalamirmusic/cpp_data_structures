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
//
//Everything here is constexpr apart from the two sort overloads: std::sort is
//not constexpr until C++26, and this library is C++20. So an Array of a literal
//type can be built, filled, indexed, searched and compared at compile time -
//which is what lets a type that stores one (a matrix, a lookup table) keep its
//own constexpr rather than lose it to its storage.
template <typename T, int Size>
class Array
{
public:
    using value_type = T;
    using size_type = int;
    using ContainerType = std::array<T, (size_t) Size>;
    using Iterator = typename ContainerType::iterator;
    using Const_Iterator = typename ContainerType::const_iterator;

    constexpr Array() = default;

    constexpr Array(std::initializer_list<T> list)
    {
        auto numToCopy = (int) list.size() < Size ? (int) list.size() : Size;
        auto out = container.begin();

        for (auto it = list.begin(); it != list.begin() + numToCopy; ++it)
            *out++ = *it;
    }

    constexpr Array(const Array& other) = default;
    constexpr Array(Array&& other) noexcept = default;

    constexpr explicit Array(const ContainerType& other) { container = other; }

    constexpr explicit Array(ContainerType&& other) noexcept
    {
        container = std::move(other);
    }

    constexpr Array& operator=(const ContainerType& other)
    {
        container = other;
        return *this;
    }

    constexpr Array& operator=(const Array& other) = default;

    constexpr bool operator==(const Array& other) const
    {
        return container == other.container;
    }

    constexpr bool operator!=(const Array& other) const
    {
        return container != other.container;
    }

    constexpr bool empty() const noexcept { return container.empty(); }

    static constexpr int size() noexcept { return Size; }

    //Const overloads included because a constexpr Array is a const one
    constexpr T& back() { return container.back(); }
    constexpr const T& back() const { return container.back(); }
    constexpr T& front() { return container.front(); }
    constexpr const T& front() const { return container.front(); }

    constexpr T& operator[](int index) noexcept { return container[(size_t) index]; }
    constexpr const T& operator[](int index) const noexcept
    {
        return container[(size_t) index];
    }
    constexpr T& get(int index) { return container[(size_t) index]; }
    constexpr const T& get(int index) const { return container[(size_t) index]; }

    constexpr Iterator begin() noexcept { return container.begin(); }
    constexpr Iterator end() noexcept { return container.end(); }

    constexpr Const_Iterator begin() const noexcept { return container.begin(); }
    constexpr Const_Iterator end() const noexcept { return container.end(); }

    constexpr Const_Iterator cbegin() const { return container.cbegin(); }
    constexpr Const_Iterator cend() const { return container.cend(); }

    constexpr bool contains(const T& element) const
    {
        return Vectors::contains(container, element);
    }

    constexpr ContainerType& getArray() { return container; }
    constexpr const ContainerType& getArray() const { return container; }
    //The source is taken by const reference, so a constant can be one
    constexpr void copyFrom(const ContainerType& other) { container = other; }
    constexpr void copyFrom(const Array& other) { container = other.getArray(); }

    template <typename A>
    constexpr void mixFrom(const A& other)
    {
        for (int index = 0; index < size(); ++index)
            container[index] += other[index];
    }

    constexpr void fill(const T& value)
    {
        for (auto& element: container)
            element = value;
    }

    template <typename A>
    void fillFrom(A& other)
    {
        Vectors::copyInto(other, container);
    }

    constexpr int getLastElementIndex() const { return size() - 1; }

    //Not constexpr: std::sort only becomes one in C++26
    void sort() { std::sort(begin(), end()); }

    template <typename A>
    constexpr int getIndexOf(const A& element) const
    {
        return Vectors::getIndexOf(container, element);
    }

    template <typename Predicate>
    constexpr int getIndexIf(Predicate&& predicate) const
    {
        return Vectors::getIndexIf(container, std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    constexpr int countIf(Predicate&& predicate) const
    {
        return Vectors::countIf(container, std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    constexpr const T* findIf(Predicate&& predicate) const
    {
        return Vectors::findIf(*this, std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    constexpr T* findIf(Predicate&& predicate)
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

    constexpr const T* data() const { return container.data(); }
    constexpr T* data() { return container.data(); }

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
