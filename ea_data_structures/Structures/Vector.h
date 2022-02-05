#pragma once

#include <vector>
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
    void reserve(int capacity) { container.reserve((size_t) capacity); }

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

    T& back() { return container.back(); }
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

    inline T& get(int index) noexcept { return container[(size_t) index]; }
    inline const T& operator[](int index) const noexcept { return get(index); }

    inline T& operator[](int index) noexcept { return get(index); }
    const T& get(int index) const noexcept { return container[(size_t) index]; }

    void clear() noexcept { container.clear(); }

    inline Iterator begin() noexcept { return container.begin(); }
    inline Iterator end() noexcept { return container.end(); }

    inline Const_Iterator begin() const noexcept { return container.begin(); }
    inline Const_Iterator end() const noexcept { return container.end(); }

    Const_Iterator cbegin() const { return container.cbegin(); }
    Const_Iterator cend() const { return container.cend(); }

    template <typename A>
    bool contains(const A& element) const
    {
        return VectorUtilities::contains(container, element);
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
        return VectorUtilities::addIfNotThere(container, element);
    }

    template <typename A>
    void removeAllMatches(const A& element)
    {
        VectorUtilities::removeAllMatches(container, element);
    }

    void resize(size_t numElements) { container.resize(numElements); }
    void resize(int numElements) { resize((size_t) numElements); }

    template <typename FloatType>
    FloatType getIndexAsRelative(int index) const
    {
        if (index < 0 || index >= size())
            return FloatType(-1);

        return Ranges::map(index,
                           FloatType(0),
                           (FloatType) getLastElementIndex(),
                           FloatType(0),
                           FloatType(1));
    }

    template <typename FloatType>
    int getRelativeIndex(FloatType proprtion) const
    {
        auto index =
            Ranges::map(proprtion, FloatType(0), (FloatType) getLastElementIndex());

        return (int) index;
    }

    template <typename FloatType>
    T& getRelative(FloatType proprtion) const
    {
        return get(getRelativeIndex(proprtion));
    }

    template <typename FloatType>
    FloatType getRelativeIndexOf(const T& item) const
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
        VectorUtilities::copyInto(other, container);
    }

    void removeRange(int startRange, int endRange)
    {
        getVector().erase(begin() + startRange, begin() + endRange);
    }

    void removeAt(int index) { VectorUtilities::removeAt(container, index); }

    template <typename Callable>
    bool eraseIf(Callable&& callable)
    {
        return VectorUtilities::eraseIf(container, callable);
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

    Vector& sort(bool forward = true)
    {
        VectorUtilities::sort(container, forward);
        return *this;
    }

    template <typename Predicate>
    Vector& sort(const Predicate& pred, bool reverse = false)
    {
        VectorUtilities::sort(container, pred, reverse);
        return *this;
    }

    Vector& reverse()
    {
        VectorUtilities::reverse(container);
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
        return VectorUtilities::getIndexOf(container, element);
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
        return VectorUtilities::transform(*this, std::forward<Func>(func));
    }

    template <typename Predicate>
    auto filter(Predicate&& predicate) const
    {
        return VectorUtilities::filter(*this, std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    Vector& filterInPlace(Predicate&& predicate)
    {
        auto removed = std::remove_if(begin(), end(), predicate);
        container.erase(removed);
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
