#pragma once

#include "StaticVector.h"
#include "Vector.h"

namespace EA
{
//A vector with small-buffer optimisation: elements live in a StaticVector of
//size PreAllocatedSize until you exceed it, at which point storage switches
//to a heap-backed Vector. isStatic() reports which mode is currently active.
template <typename T, int PreAllocatedSize>
struct SmallVector : VectorBase
{
    using value_type = T;
    using Iterator = T*;

    //The same kind of container holding a different element type,
    //used by Vectors::transform
    template <typename U>
    using Rebound = SmallVector<U, PreAllocatedSize>;

    SmallVector() = default;
    SmallVector(std::initializer_list<T> list) { add(list); }

    bool empty() const noexcept { return size() == 0; }
    int size() const noexcept
    {
        if (isStatic())
            return staticVec.size();

        return dynamicVec.size();
    }

    void insert(int position, const T& object)
    {
        checkSwitch(1);

        if (isStatic())
            staticVec.insert(position, object);
        else
            dynamicVec.insert(position, object);
    }

    bool isStatic() const { return usingStatic; }

    bool shouldSwitch(int sizeExtra) const
    {
        if (isStatic())
            return size() + sizeExtra > PreAllocatedSize;

        return false;
    }

    void checkSwitch(int sizeExtra)
    {
        if (shouldSwitch(sizeExtra))
            switchToDynamic(sizeExtra);
    }

    void switchToDynamic(int sizeExtra)
    {
        dynamicVec.reserve(size() + sizeExtra);

        for (auto& element: *this)
            dynamicVec.create(std::move(element));

        usingStatic = false;
        staticVec.clear();
    }

    T& back() { return get(getLastElementIndex()); }
    T& front() { return get(0); }

    T& add(const T& elementToAdd) noexcept
    {
        checkSwitch(1);

        if (isStatic())
            return staticVec.add(elementToAdd);

        return dynamicVec.add(elementToAdd);
    }

    T& push_back(const T& elementToAdd) noexcept { return add(elementToAdd); }

    T& add(T&& elementToAdd) noexcept
    {
        checkSwitch(1);

        if (isStatic())
            return staticVec.add(std::move(elementToAdd));

        return dynamicVec.add(std::move(elementToAdd));
    }

    void add(std::initializer_list<T> items) noexcept
    {
        checkSwitch((int) items.size());

        if (isStatic())
            staticVec.add(items);
        else
            dynamicVec.add(items);
    }

    template <typename... Args>
    T& create(Args&&... args)
    {
        checkSwitch(1);

        if (isStatic())
            return staticVec.create(std::forward<Args>(args)...);

        return dynamicVec.create(std::forward<Args>(args)...);
    }

    template <typename... Args>
    T& emplace_back(Args&&... args)
    { return create(std::forward<Args>(args)...); }

    T& get(int index) noexcept { return data()[index]; }
    const T& operator[](int index) const noexcept { return get(index); }

    T& operator[](int index) noexcept { return get(index); }
    const T& get(int index) const noexcept { return data()[index]; }

    void clear() noexcept
    {
        if (isStatic())
            staticVec.clear();
        else
            dynamicVec.clear();
    }

    T* begin() noexcept { return data(); }
    T* end() noexcept { return data() + size(); }

    const T* begin() const noexcept { return data(); }
    const T* end() const noexcept { return data() + size(); }

    const T* cbegin() const noexcept { return data(); }
    const T* cend() const noexcept { return data() + size(); }

    template <typename A>
    bool contains(const A& element) const
    { return Vectors::contains(*this, element); }

    void copyFrom(const SmallVector& other)
    {
        usingStatic = other.usingStatic;

        if (other.usingStatic)
            staticVec = other.staticVec;
        else
            dynamicVec = other.dynamicVec;
    }

    void copyFrom(const SmallVector& other, int startIndex, int numItems)
    { Vectors::copyRange(*this, other, startIndex, numItems); }

    void copyFrom(const SmallVector& other, int numItems)
    { copyFrom(other, 0, numItems); }

    bool addIfNotThere(const T& element)
    { return Vectors::addIfNotThere(*this, element); }

    template <typename A>
    int removeAllMatches(const A& element)
    { return Vectors::removeAllMatches(*this, element); }

    void resize(size_t numElements) { resize((int) numElements); }
    void resize(int numElements)
    {
        checkSwitch(numElements - size());

        if (isStatic())
            staticVec.resize(numElements);
        else
            dynamicVec.resize(numElements);
    }

    template <typename FloatType>
    FloatType getIndexAsRelative(int index) const
    { return Vectors::getIndexAsRelative<FloatType>(*this, index); }

    template <typename FloatType>
    int getRelativeIndex(FloatType proportion) const
    { return Vectors::getRelativeIndex(*this, proportion); }

    template <typename FloatType>
    T& getRelative(FloatType proportion)
    { return Vectors::getRelative(*this, proportion); }

    template <typename FloatType>
    const T& getRelative(FloatType proportion) const
    { return Vectors::getRelative(*this, proportion); }

    template <typename FloatType>
    FloatType getRelativeIndexOf(const T& item) const
    { return Vectors::getRelativeIndexOf<FloatType>(*this, item); }

    template <typename... Args>
    void resizeAndCreate(int numElements, Args&&... args)
    {
        checkSwitch(numElements - size());

        if (isStatic())
            staticVec.resizeAndCreate(numElements, std::forward<Args>(args)...);
        else
            dynamicVec.resizeAndCreate(numElements, std::forward<Args>(args)...);
    }

    template <typename A>
    void mixFrom(A& other)
    { Vectors::mixFrom(*this, other); }

    void fill(const T& value) { Vectors::fill(*this, value); }

    void fill(const T& value, int numItems)
    { Vectors::fill(*this, value, numItems); }

    template <typename A>
    void addFrom(const A& other)
    { Vectors::addFrom(*this, other); }

    template <typename A>
    void addFromIndexes(const A& other, std::initializer_list<int> indexes)
    { Vectors::addFromIndexes(*this, other, indexes); }

    template <typename A>
    void fillFrom(A& other)
    { Vectors::copyInto(other, *this); }

    void erase(Iterator it) { removeAt(int(it - begin())); }

    void removeRange(int startRange, int endRange)
    { Vectors::removeRange(*this, startRange, endRange); }

    void removeAt(int index)
    {
        if (isStatic())
            staticVec.removeAt(index);
        else
            dynamicVec.removeAt(index);
    }

    template <typename Callable>
    bool eraseIf(Callable&& callable)
    { return Vectors::eraseIf(*this, callable); }

    void pop_back()
    {
        if (!empty())
            removeAt(getLastElementIndex());
    }

    int getLastElementIndex() const noexcept { return size() - 1; }
    int getLastValidElementIndex() const noexcept
    { return std::max(0, getLastElementIndex()); }

    SmallVector& sort(bool forward = true)
    {
        Vectors::sort(*this, forward);
        return *this;
    }

    template <typename Predicate>
    SmallVector& sort(const Predicate& pred, bool forward = true)
    {
        Vectors::sort(*this, pred, forward);
        return *this;
    }

    SmallVector& reverse()
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
    { return Vectors::getIndexOf(*this, element); }

    template <typename ObjectType>
    const T* find(const ObjectType& element) const
    { return Vectors::find(*this, element); }

    template <typename ObjectType>
    T* find(const ObjectType& element)
    { return Vectors::find(*this, element); }

    template <typename Func>
    auto transform(Func&& func) const
    { return Vectors::transform(*this, std::forward<Func>(func)); }

    template <typename Predicate>
    auto filter(Predicate&& predicate) const
    { return Vectors::filter(*this, std::forward<Predicate>(predicate)); }

    template <typename Predicate>
    SmallVector& filterInPlace(Predicate&& predicate)
    {
        Vectors::eraseIf(*this, predicate);
        return *this;
    }

    template <typename Predicate>
    void copyFilteredTo(SmallVector& other, Predicate&& predicate) const
    { Vectors::copyFilteredTo(*this, other, predicate); }

    template <typename Predicate>
    void addFilteredTo(SmallVector& other, Predicate&& predicate) const
    { Vectors::addFilteredTo(*this, other, predicate); }

    const T* data() const
    {
        if (usingStatic)
            return staticVec.data();

        return dynamicVec.data();
    }

    T* data()
    {
        if (usingStatic)
            return staticVec.data();

        return dynamicVec.data();
    }

    Vector<T> dynamicVec;
    StaticVector<T, PreAllocatedSize> staticVec;
    bool usingStatic = true;
};
} // namespace EA