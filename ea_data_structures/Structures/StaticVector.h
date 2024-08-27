#pragma once

#include "../ValueWrapper/Constructed.h"
#include "Vector.h"

namespace EA
{
template <typename T, int MaxSize>
struct StaticVector : VectorBase
{
    using ContainerType = Array<RawStorage<T>, MaxSize>;
    using value_type = T;
    using Iterator = T*;
    using ConstIterator = const T*;

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
        if (currentSize < MaxSize)
        {
            std::rotate(begin() + position, container.end() - 1, container.end());
            container[position] = object;
            ++currentSize;
        }
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
            container[currentSize].create(elementToAdd);
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
        for (int index = 0; index < size(); ++index)
        {
            if (get(index) == element)
                return true;
        }

        return  false;
    }

    ContainerType& getVector() { return container; }

    bool addIfNotThere(const T& element)
    {
        return Vectors::addIfNotThere(*this, element);
    }

    template <typename A>
    int removeAllMatches(const A& element)
    {
        int removedElements = 0;

        for (int index = getLastElementIndex(); index >= 0; --index)
        {
            if (get(index) == element)
            {
                ++removedElements;
                removeAt(index);
            }
        }

        return removedElements;
    }

    void resize(size_t numElements) { resize((int) numElements); }
    void resize(int numElements)
    {
        numElements = std::min(MaxSize, numElements);

        if (numElements < currentSize)
        {
            for (int index = getLastElementIndex(); index >= 0; --index)
                container[index].destroy();
        }
        else if (numElements > currentSize)
        {
            for (int index = 0; index < numElements; ++index)
                container[index].create();
        }

        currentSize = numElements;
    }

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
        numElements = std::min(MaxSize, numElements);

        if (numElements < currentSize)
        {
            for (int index = getLastElementIndex(); index >= 0; --index)
                container[index].destroy();
        }
        else if (numElements > currentSize)
        {
            for (int index = 0; index < numElements; ++index)
                container[index].create(std::forward<Args>(args)...);
        }

        currentSize = numElements;
    }

    template <typename A>
    void mixFrom(A& other)
    {
        for (int index = 0; index < size(); ++index)
            get(index) += other[index];
    }

    void fill(const T& value)
    {
        for (auto& element: *this)
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
        for (auto& element: other)
            push_back(element);
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
        getVector().erase(begin() + startRange, begin() + endRange);
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
        bool erased = false;
        auto last = getLastElementIndex();

        for (int index = last; index >= 0; --index)
        {
            if (callable(get(index)))
            {
                removeAt(index);
                erased = true;
            }
        }

        return erased;
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
    StaticVector& sort(const Predicate& pred, bool reverse = false)
    {
        Vectors::sort(*this, pred, reverse);
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
        return Vectors::filter(*this, std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    StaticVector& filterInPlace(Predicate&& predicate)
    {
        auto removed = std::remove_if(begin(), end(), predicate);
        resize(removed - begin());
        return *this;
    }

    template <typename Predicate>
    void copyFilteredTo(StaticVector& other, Predicate&& predicate) const
    {
        std::copy_if(begin(), end(), other.begin(), predicate);
    }

    template <typename Predicate>
    void addFilteredTo(StaticVector& other, Predicate&& predicate) const
    {
        std::copy_if(begin(), end(), std::back_inserter(other), predicate);
    }

    const T* data() const { return reinterpret_cast<const T*>(container.data()); }
    T* data() { return reinterpret_cast<T*>(container.data()); }

    int currentSize = 0;
    ContainerType container;
};
} // namespace EA