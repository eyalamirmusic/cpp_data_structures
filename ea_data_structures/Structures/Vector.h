#pragma once

#include <vector>
#include <iterator>
#include "SizeType.h"
#include "../Utilities/VectorUtilities.h"

namespace EA
{
//Empty base class to allow checking if it's a vector
struct VectorBase
{
};

template <typename T>
constexpr bool isVector()
{
    return std::is_base_of_v<VectorBase, T>;
}

template <typename T, typename Allocator = std::allocator<T>>
class Vector : VectorBase
{
public:
    using ContainerType = std::vector<T, Allocator>;
    using size_type = int;
    using Iterator = typename ContainerType::iterator;
    using Const_Iterator = typename ContainerType::const_iterator;
    using value_type = T;

    Vector() = default;
    Vector(const Vector& other)
        : container(other.container)
    {
    }
    Vector(Vector&& other) noexcept
        : container(std::move(other.container))
    {
    }

    explicit Vector(const ContainerType& other)
        : container(other)
    {
    }

    Vector(std::initializer_list<T> list)
        : container(list)
    {
    }

    explicit Vector(int numItems) { resize(numItems); }

    int capacity() const { return (int) container.capacity(); }
    void reserve(SizeType capacity) { container.reserve(capacity); }

    void reserveAtLeast(int capacityToUse)
    {
        if (capacity() < capacityToUse)
            reserve(capacityToUse);
    }

    Vector& operator=(const Vector& other)
    {
        container = other.container;
        return *this;
    }

    Vector& operator=(std::initializer_list<T> list)
    {
        container = list;
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept
    {
        container = std::move(other.container);
        return *this;
    }

    Vector& operator=(const ContainerType& other)
    {
        container = other;
        return *this;
    }

    bool empty() const noexcept { return container.empty(); }

    int size() const noexcept { return (int) container.size(); }

    void insert(int position, const T& object)
    {
        container.insert(begin() + position, object);
    }

    template <typename... Args>
    T& insertAt(int position, Args&&... args)
    {
        container.emplace(begin() + position, std::forward<Args>(args)...);

        return get(position);
    }

    template <typename... Args>
    void insertRange(int start, int numItems, Args&&... args)
    {
        while (numItems > 0)
        {
            insertAt(start, std::forward<Args>(args)...);
            --numItems;
        }
    }

    const T& back() const { return container.back(); }
    T& back() { return container.back(); }

    const T& front() const { return container.front(); }
    T& front() { return container.front(); }

    T& add(const T& elementToAdd) noexcept
    {
        container.push_back(elementToAdd);
        return back();
    }

    T& push_back(const T& elementToAdd) noexcept { return add(elementToAdd); }

    template <typename Iterator>
    void erase(Iterator it)
    {
        container.erase(it);
    }

    template <typename Iterator>
    void erase(Iterator first, Iterator last)
    {
        container.erase(first, last);
    }

    T& add(T&& elementToAdd) noexcept
    {
        container.push_back(std::move(elementToAdd));
        return back();
    }

    void add(std::initializer_list<T> items) noexcept
    {
        for (auto& item: items)
            add(item);
    }

    template <typename... Args>
    T& create(Args&&... args)
    {
        container.emplace_back(std::forward<Args>(args)...);
        return back();
    }

    template <typename... Args>
    T& emplace_back(Args&&... args)
    {
        return create(std::forward<Args>(args)...);
    }

    T& get(SizeType index) noexcept { return container[index]; }
    const T& operator[](SizeType index) const noexcept { return get(index); }

    T& operator[](SizeType index) noexcept { return get(index); }
    const T& get(SizeType index) const noexcept { return container[index]; }

    void clear() noexcept { container.clear(); }

    auto rbegin() noexcept { return container.rbegin(); }
    auto rend() noexcept { return container.rend(); }

    auto rbegin() const noexcept { return container.rbegin(); }
    auto rend() const noexcept { return container.rend(); }

    auto begin() noexcept { return container.begin(); }
    auto end() noexcept { return container.end(); }

    auto begin() const noexcept { return container.begin(); }
    auto end() const noexcept { return container.end(); }

    auto cbegin() const { return container.cbegin(); }
    auto cend() const { return container.cend(); }

    template <typename A>
    bool contains(const A& element) const
    {
        return Vectors::contains(container, element);
    }

    ContainerType& getVector() { return container; }
    void copyFrom(ContainerType& other) { container = other; }
    void copyFrom(Vector& other) { container = other.getVector(); }

    void copyFrom(const Vector& other, int startIndex, int numItems)
    {
        auto targetSize = numItems - startIndex;
        auto adjustedSize = std::min(targetSize, other.size());

        reserveAtLeast(adjustedSize);
        clear();

        for (int index = startIndex; index < startIndex; ++index)
            add(other[index]);
    }

    void copyFrom(const Vector& other, int numItems)
    {
        copyFrom(other, 0, numItems);
    }

    bool addIfNotThere(const T& element)
    {
        return Vectors::addIfNotThere(container, element);
    }

    template <typename A>
    int removeAllMatches(const A& element)
    {
        return Vectors::removeAllMatches(container, element);
    }

    template <typename InputIterator>
    void assign(InputIterator first, InputIterator last)
    {
        container.assign(first, last);
    }

    void assign(SizeType n, const T& val) { container.assign(n, val); }

    void resize(SizeType elements, const T& value)
    {
        container.resize(elements, value);
    }

    void resize(SizeType numElements) { container.resize(numElements); }

    template <typename FloatType>
    FloatType getIndexAsRelative(int index) const
    {
        if (index < 0 || index >= size())
            return FloatType(-1);

        return Ranges::map(FloatType(index),
                           FloatType(0),
                           (FloatType) getLastElementIndex(),
                           FloatType(0),
                           FloatType(1));
    }

    template <typename FloatType>
    int getRelativeIndex(FloatType proprtion) const
    {
        return Ranges::getIndexProprtion(proprtion, size());
    }

    template <typename Callable>
    Vector<int> getIndexesMatching(Callable&& func) const
    {
        Vector<int> indexes;

        for (int index = 0; index < size(); ++index)
        {
            if (func(get(index)))
                indexes.add(index);
        }

        return indexes;
    }

    template <typename Callable>
    void removeIndexesMatching(Callable&& func)
    {
        auto indexes = getIndexesMatching(func);
        removeIndexes(indexes);
    }

    template <typename FloatType>
    T& getRelative(FloatType proprtion)
    {
        return get(getRelativeIndex(proprtion));
    }

    template <typename FloatType>
    const T& getRelative(FloatType proprtion) const
    {
        return get(getRelativeIndex(proprtion));
    }

    template <typename FloatType, typename A>
    FloatType getRelativeIndexOf(const A& item) const
    {
        return getIndexAsRelative<FloatType>(getIndexOf(item));
    }

    template <typename... Args>
    void resizeAndCreate(int numElements, Args&&... args)
    {
        resize(numElements);

        for (auto& element: container)
        {
            if (element == nullptr)
                element.create(std::forward<Args>(args)...);
        }
    }

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

    void fill(const T& value, int numItems)
    {
        for (int index = 0; index < numItems; ++index)
            get(index) = value;
    }

    template <typename A>
    void addFrom(const A& other)
    {
        reserveAtLeast(size() + other.size());

        for (auto& element: other)
            container.push_back(element);
    }

    template <typename A>
    void addFromIndexes(const A& other, std::initializer_list<int> indexes)
    {
        for (auto& index: indexes)
            add(other[index]);
    }

    template <typename A>
    void fillFrom(A& other)
    {
        Vectors::copyInto(other, container);
    }

    void removeRange(int startRange, int endRange)
    {
        if (!empty() && endRange < size())
            getVector().erase(begin() + startRange, begin() + endRange);
    }

    void removeAt(int index) { Vectors::removeAt(container, index); }

    template <typename A>
    void removeIndexes(A& indexes)
    {
        indexes.sort(false);

        for (auto i: indexes)
            removeAt(i);
    }

    template <typename Callable>
    bool eraseIf(Callable&& callable)
    {
        return Vectors::eraseIf(container, callable);
    }

    void pop_back()
    {
        if (!empty())
            container.pop_back();
    }

    int getLastElementIndex() const noexcept { return size() - 1; }
    int getLastValidElementIndex() const noexcept
    {
        return std::max(0, getLastElementIndex());
    }

    Vector& stableSort(bool forward = true)
    {
        Vectors::stableSort(container, forward);
        return *this;
    }

    template <typename Func>
    Vector& stableSort(Func&& func, bool forward = true)
    {
        Vectors::stableSort(container, func, forward);
        return *this;
    }

    Vector& sort(bool forward = true)
    {
        Vectors::sort(container, forward);
        return *this;
    }

    template <typename Predicate>
    Vector& sort(const Predicate& pred, bool forward = true)
    {
        Vectors::sort(container, pred, forward);
        return *this;
    }

    bool shift(int offset) noexcept
    {
        if (offset > 0)
            std::rotate(rbegin(), rbegin() + offset, rend());
        else if (offset < 0)
            std::rotate(begin(), begin() - offset, end());

        return offset != 0;
    }

    Vector& reverse()
    {
        Vectors::reverse(container);
        return *this;
    }

    //Finds the index by comparing the element
    //Notice that if this is a vector of pointers (OwnedVecor), this will compare
    //The addresses of objects
    //
    //Also see OwnedVector helper functions for special cases
    template <typename ObjectType>
    int getIndexOf(const ObjectType& element) const
    {
        return Vectors::getIndexOf(container, element);
    }

    template <typename ObjectType>
    const T* find(const ObjectType& element) const
    {
        auto index = getIndexOf(element);

        if (index >= 0)
            return &get(index);

        return nullptr;
    }

    template <typename ObjectType>
    T* find(const ObjectType& element)
    {
        auto index = getIndexOf(element);

        if (index >= 0)
            return &get(index);

        return nullptr;
    }

    template <typename Func>
    auto transform(Func&& func) const
    {
        return Vectors::transform(*this, std::forward<Func>(func));
    }

    template <typename Predicate>
    auto filter(Predicate&& predicate) const
    {
        return Vectors::filter(*this, predicate);
    }

    template <typename Predicate>
    Vector& filterInPlace(Predicate&& predicate)
    {
        auto removed = std::remove_if(begin(), end(), predicate);
        container.erase(removed, end());
        return *this;
    }

    template <typename Predicate>
    void copyFilteredTo(Vector& other, Predicate&& predicate) const
    {
        std::copy_if(begin(), end(), other.begin(), predicate);
    }

    template <typename Predicate>
    void addFilteredTo(Vector& other, Predicate&& predicate) const
    {
        std::copy_if(begin(), end(), std::back_inserter(other), predicate);
    }

    const T* data() const { return container.data(); }
    T* data() { return container.data(); }

protected:
    ContainerType container;
};

template <typename T>
using NestedVector = Vector<Vector<T>>;

} // namespace EA
