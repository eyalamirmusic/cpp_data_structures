#pragma once

#include "../ValueWrapper/RawStorage.h"
#include "Array.h"
#include "Vector.h"

namespace EA
{
//A vector-like container with a compile-time maximum capacity and no heap
//allocation: elements live in-place inside an Array<RawStorage<T>, MaxSize>.
//Pushing past MaxSize is a no-op. Shares most of Vector's helper API.
template <typename T, int MaxSize>
struct StaticVector : VectorBase
{
    using ContainerType = Array<RawStorage<T>, MaxSize>;
    using value_type = T;
    using Iterator = T*;
    using ConstIterator = const T*;

    //The same kind of container holding a different element type,
    //used by Vectors::transform
    template <typename U>
    using Rebound = StaticVector<U, MaxSize>;

    StaticVector() = default;
    StaticVector(std::initializer_list<T> list) { add(list); }

    StaticVector(const StaticVector& other) { copyFrom(other); }

    StaticVector& operator=(const StaticVector& other)
    {
        copyFrom(other);
        return *this;
    }

    void copyFrom(const StaticVector& other)
    {
        clear();

        for (auto& element: other)
            add(element);
    }

    ~StaticVector()
    {
        for (int index = 0; index < currentSize; ++index)
            container[index].destroy();
    }

    bool empty() const noexcept { return currentSize == 0; }
    int size() const noexcept { return currentSize; }

    void insert(int position, const T& object)
    {
        if (currentSize >= MaxSize)
            return;

        if (position >= currentSize)
        {
            add(object);
            return;
        }

        //Make room by move-constructing a new last element from the current
        //last one, then shifting the rest one slot to the right
        container[currentSize].create(std::move(*container[currentSize - 1]));

        for (int index = currentSize - 1; index > position; --index)
            get(index) = std::move(get(index - 1));

        get(position) = object;
        ++currentSize;
    }

    T& back() { return get(getLastElementIndex()); }
    T& front() { return get(0); }

    T& add(const T& elementToAdd)
    {
        if (currentSize < MaxSize)
        {
            container[currentSize].create(elementToAdd);
            ++currentSize;
        }

        return back();
    }

    T& push_back(const T& elementToAdd) noexcept { return add(elementToAdd); }

    T& add(T&& elementToAdd) noexcept
    {
        if (currentSize < MaxSize)
        {
            container[currentSize].create(std::move(elementToAdd));
            ++currentSize;
        }

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
        if (currentSize < MaxSize)
        {
            container[currentSize].create(std::forward<Args>(args)...);
            ++currentSize;
        }

        return back();
    }

    template <typename... Args>
    T& emplace_back(Args&&... args)
    {
        return create(std::forward<Args>(args)...);
    }

    T& get(int index) noexcept { return *container[index]; }
    const T& operator[](int index) const noexcept { return get(index); }

    T& operator[](int index) noexcept { return get(index); }
    const T& get(int index) const noexcept { return data()[index]; }

    void clear() noexcept
    {
        for (int index = 0; index < currentSize; ++index)
            container[index].destroy();

        currentSize = 0;
    }

    Iterator begin() noexcept { return data(); }
    Iterator end() noexcept { return data() + currentSize; }

    ConstIterator begin() const noexcept { return data(); }
    ConstIterator end() const noexcept { return data() + currentSize; }

    template <typename A>
    bool contains(const A& element) const
    {
        return Vectors::contains(*this, element);
    }

    ContainerType& getVector() { return container; }

    bool addIfNotThere(const T& element)
    {
        return Vectors::addIfNotThere(*this, element);
    }

    template <typename A>
    int removeAllMatches(const A& element)
    {
        return Vectors::removeAllMatches(*this, element);
    }

    void resize(size_t numElements) { resize((int) numElements); }
    void resize(int numElements)
    {
        numElements = std::min(MaxSize, numElements);

        if (numElements < currentSize)
        {
            for (int index = currentSize - 1; index >= numElements; --index)
                container[index].destroy();
        }
        else if (numElements > currentSize)
        {
            for (int index = currentSize; index < numElements; ++index)
                container[index].create();
        }

        currentSize = numElements;
    }

    template <typename FloatType>
    FloatType getIndexAsRelative(int index) const
    {
        return Vectors::getIndexAsRelative<FloatType>(*this, index);
    }

    template <typename FloatType>
    int getRelativeIndex(FloatType proportion) const
    {
        return Vectors::getRelativeIndex(*this, proportion);
    }

    template <typename FloatType>
    T& getRelative(FloatType proportion)
    {
        return Vectors::getRelative(*this, proportion);
    }

    template <typename FloatType>
    const T& getRelative(FloatType proportion) const
    {
        return Vectors::getRelative(*this, proportion);
    }

    template <typename FloatType>
    FloatType getRelativeIndexOf(const T& item) const
    {
        return Vectors::getRelativeIndexOf<FloatType>(*this, item);
    }

    template <typename... Args>
    void resizeAndCreate(int numElements, Args&&... args)
    {
        numElements = std::min(MaxSize, numElements);

        if (numElements < currentSize)
        {
            for (int index = currentSize - 1; index >= numElements; --index)
                container[index].destroy();
        }
        else if (numElements > currentSize)
        {
            for (int index = currentSize; index < numElements; ++index)
                container[index].create(std::forward<Args>(args)...);
        }

        currentSize = numElements;
    }

    template <typename A>
    void mixFrom(A& other)
    {
        Vectors::mixFrom(*this, other);
    }

    void fill(const T& value) { Vectors::fill(*this, value); }

    void fill(const T& value, int numItems)
    {
        Vectors::fill(*this, value, numItems);
    }

    template <typename A>
    void addFrom(const A& other)
    {
        Vectors::addFrom(*this, other);
    }

    template <typename A>
    void addFromIndexes(const A& other, std::initializer_list<int> indexes)
    {
        Vectors::addFromIndexes(*this, other, indexes);
    }

    template <typename A>
    void fillFrom(A& other)
    {
        Vectors::copyInto(other, *this);
    }

    void copyFrom(const StaticVector& other, int startIndex, int numItems)
    {
        Vectors::copyRange(*this, other, startIndex, numItems);
    }

    void copyFrom(const StaticVector& other, int numItems)
    {
        copyFrom(other, 0, numItems);
    }

    void removeRange(int startRange, int endRange)
    {
        Vectors::removeRange(*this, startRange, endRange);
    }

    void erase(Iterator it) { removeAt(it - begin()); }

    void removeAt(int index)
    {
        if (currentSize >= 1 && index >= 0 && index < currentSize)
        {
            for (int elementIndex = index; elementIndex < size(); ++elementIndex)
            {
                container[elementIndex].destroy();

                if (elementIndex < getLastElementIndex())
                {
                    container[elementIndex].create(
                        std::move(container[elementIndex + 1].object));
                }
            }

            --currentSize;
        }
    }

    template <typename Callable>
    bool eraseIf(Callable&& callable)
    {
        return Vectors::eraseIf(*this, callable);
    }

    void pop_back()
    {
        if (!empty())
            removeAt(getLastElementIndex());
    }

    int getLastElementIndex() const noexcept { return size() - 1; }
    int getLastValidElementIndex() const noexcept
    {
        return std::max(0, getLastElementIndex());
    }

    StaticVector& sort(bool forward = true)
    {
        Vectors::sort(*this, forward);
        return *this;
    }

    template <typename Predicate>
    StaticVector& sort(const Predicate& pred, bool forward = true)
    {
        Vectors::sort(*this, pred, forward);
        return *this;
    }

    StaticVector& reverse()
    {
        Vectors::reverse(*this);
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
        return Vectors::getIndexOf(*this, element);
    }

    template <typename ObjectType>
    const T* find(const ObjectType& element) const
    {
        return Vectors::find(*this, element);
    }

    template <typename ObjectType>
    T* find(const ObjectType& element)
    {
        return Vectors::find(*this, element);
    }

    template <typename Func>
    auto transform(Func&& func) const
    {
        return Vectors::transform(*this, std::forward<Func>(func));
    }

    template <typename Predicate>
    auto filter(Predicate&& predicate) const
    {
        return Vectors::filter(*this, std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    StaticVector& filterInPlace(Predicate&& predicate)
    {
        Vectors::eraseIf(*this, predicate);
        return *this;
    }

    template <typename Predicate>
    void copyFilteredTo(StaticVector& other, Predicate&& predicate) const
    {
        Vectors::copyFilteredTo(*this, other, predicate);
    }

    template <typename Predicate>
    void addFilteredTo(StaticVector& other, Predicate&& predicate) const
    {
        Vectors::addFilteredTo(*this, other, predicate);
    }

    const T* data() const { return reinterpret_cast<const T*>(container.data()); }
    T* data() { return reinterpret_cast<T*>(container.data()); }

    int currentSize = 0;
    ContainerType container;
};
} // namespace EA