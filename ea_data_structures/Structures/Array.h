#pragma once

//A wrapper around std::array that uses int instead of size_t, and adds some useful helper functions

#include <array>

namespace EA
{
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
        : container(list)
    {
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

    Array& operator=(const Array& other)
    {
        container = other.container;
        return *this;
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

    template<typename A>
    int getIndexOf(const A& element) const
    {
        return Vectors::getIndexOf(container, element);
    }

    template <typename Predicate>
    void sort(const Predicate& pred, bool forward = true)
    {
        std::sort(begin(), end(), pred);

        if (!forward)
            std::reverse(begin(), end());
    }

    const T* data() const { return container.data(); }
    T* data() { return container.data(); }

protected:
    ContainerType container {};
};
} // namespace EA
