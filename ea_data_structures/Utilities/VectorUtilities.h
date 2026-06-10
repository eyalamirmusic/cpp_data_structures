/*
created by: Eyal Amir
eyalamirmusic@gmail.com

These are simple templated vector/array operations meant for simplifying actions
happen often.

Such as: Add only if item isn't there, remove if there, etc.

These will work for times where T is a simple type, or a type for which operator
"==" can be evaluated correctly.

To use, just include this header and call:
Vectors::addIfNotThere(myVector, element).

 */

#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <vector>

namespace EA::Ranges
{
template <typename T>
constexpr T linearMap(T sourceValue,
                      T sourceRangeMin,
                      T sourceRangeMax,
                      T targetRangeMin,
                      T targetRangeMax) noexcept
{
    return targetRangeMin
           + (targetRangeMax - targetRangeMin) * (sourceValue - sourceRangeMin)
                 / (sourceRangeMax - sourceRangeMin);
}

template <typename T>
constexpr T map(T value,
                T sourceRangeMin,
                T sourceRangeMax,
                T targetRangeMin,
                T targetRangeMax) noexcept
{
    auto normalized = linearMap(value, sourceRangeMin, sourceRangeMax, T(0), T(1));

    if (targetRangeMax < targetRangeMin)
    {
        return linearMap(
            T(1) - normalized, T(0), T(1), targetRangeMax, targetRangeMin);
    }

    return linearMap(
        value, sourceRangeMin, sourceRangeMax, targetRangeMin, targetRangeMax);
}

template <typename T>
constexpr T map(T value0To1, T targetRangeMin, T targetRangeMax) noexcept
{ return linearMap(value0To1, T(0), T(1), targetRangeMin, targetRangeMax); }

template <typename T>
constexpr T mapTwoWayNormalized(T value, T ratio)
{
    if (ratio > T(0.5))
    {
        ratio = Ranges::map(ratio, (T) 0.5, (T) 1, T(0), T(1));
        return Ranges::map(ratio, value, (T) 1);
    }

    if (ratio < T(0.5))
    {
        ratio = Ranges::map(ratio, (T) 0, (T) 0.5, (T) 0, (T) 1);
        return Ranges::map(ratio, (T) 0, value);
    }

    return value;
}

template <typename FloatType, typename SizeType>
auto getIndexProprtion(FloatType proportion, SizeType size) noexcept
{
    if (proportion == static_cast<FloatType>(1))
        return size - 1;

    return static_cast<SizeType>(proportion * (FloatType) size);
}

} // namespace EA::Ranges

namespace EA::Vectors
{
// Gets the index of an element that can be compared to each element of the
// container If it's not found, it will return -1;
template <typename T, typename Func>
int getIndexOfComparison(const T& container, Func&& comparisonFunc)
{
    int index = 0;

    for (const auto& element: container)
    {
        if (comparisonFunc(element))
            return index;

        ++index;
    }

    return -1;
}

// Gets the index of an element that can be compared to each element of the
// container If it's not found, it will return -1;
template <typename T, typename A>
int getIndexOf(const T& container, const A& element)
{
    auto it = std::ranges::find(container, element);

    if (it == std::ranges::end(container))
        return -1;

    return static_cast<int>(
        std::ranges::distance(std::ranges::begin(container), it));
}

template <typename T, typename A>
int getIndexOfReverse(const T& container, const A& element)
{
    return getIndexOfComparison(container,
                                [&](const auto& e) { return element == e; });
}

/** Gets the index of the first element which satisfies the given predicate
 *  Returns -1 if no elements in the container match
 */
template <typename ContainerType, typename F>
int getIndexIf(const ContainerType& container, F&& predicate)
{
    int index = 0;

    for (auto& element: container)
    {
        if (predicate(element))
            return index;
        ++index;
    }

    return -1;
}

template <typename T>
void reverse(T& container)
{ std::ranges::reverse(container); }

template <typename T, typename Func>
void stableSort(T& container, Func&& func, bool forward = true)
{
    std::stable_sort(container.begin(), container.end(), std::forward<Func>(func));

    if (!forward)
        reverse(container);
}

template <typename T>
void stableSort(T& container, bool forward = true)
{
    std::stable_sort(container.begin(), container.end());

    if (!forward)
        reverse(container);
}

template <typename T>
void sort(T& container, bool forward = true)
{
    std::sort(container.begin(), container.end());

    if (!forward)
        reverse(container);
}

template <typename T, typename COMPARE>
void sort(T& container, COMPARE compare, bool forward = true)
{
    std::sort(container.begin(), container.end(), compare);

    if (!forward)
        reverse(container);
}

// Check if an element that be compared to elements of this container exist.
template <typename T, typename A>
bool contains(const T& container, const A& elementToCheck)
{
    return std::ranges::find(container, elementToCheck)
           != std::ranges::end(container);
}

// Gets a pointer to the first element comparing equal to the given value.
// If no element matches, this returns a nullptr
template <typename Container, typename A>
auto find(Container& container, const A& element)
{
    using Pointer = decltype(&container[0]);

    auto index = getIndexOf(container, element);

    if (index >= 0)
        return Pointer(&container[index]);

    return Pointer(nullptr);
}

// Gets a pointer to an element that can be compared to an element of this
// container, If the element doesn't exist, this returns a nullptr
template <typename Container, typename A>
auto getElementPointer(Container& container, const A& element)
{ return find(container, element); }

// Gets a reference to an element that can be compared to an element of this
// container. This will crash if an element isn't found!
//
// If 'not finding' the element is a valid result, use find()
// instead and check for nullptr
template <typename Container, typename A>
auto& getElementRef(Container& container, const A& element)
{ return *find(container, element); }

// Remove an element of the container at a certain index.
// If this is called in a loop that iterates over the container, make sure you
// do it in a reversed order
//  (Starting from the end and going back to 0)
template <typename T>
void removeAt(T& container, int index)
{
    if (index >= 0 && index < (int) container.size())
        container.erase(container.begin() + index);
}

template <typename Container, typename Callable>
bool eraseIf(Container& container, Callable callable)
{
    if constexpr (requires { container.erase(container.begin(), container.end()); })
    {
        auto prevSize = container.size();
        auto removed = std::ranges::remove_if(container, callable);
        container.erase(removed.begin(), removed.end());

        return prevSize != container.size();
    }
    else
    {
        bool erased = false;

        for (int index = (int) container.size() - 1; index >= 0; --index)
        {
            if (callable(container[index]))
            {
                removeAt(container, index);
                erased = true;
            }
        }

        return erased;
    }
}

// Removed the first match found in the container, going from beginning to end.
// If there are no elements found, it will do nothing
template <typename T, typename A>
void removeFirstMatch(T& container, A& elementToCheck)
{
    auto index = getIndexOf(container, elementToCheck);

    if (index >= 0)
        removeAt(container, index);
}

// Removes all matches of an element in the container
template <typename T, typename A>
int removeAllMatches(T& container, A& elementToCheck)
{
    int removedElements = 0;

    auto numElements = (int) container.size();

    for (int index = numElements - 1; index >= 0; --index)
    {
        if (*(container.begin() + index) == elementToCheck)
        {
            ++removedElements;
            removeAt(container, index);
        }
    }

    return removedElements;
}

// Adds the element at the end only if doesn't already exist in the container
template <typename T, typename A>
bool addIfNotThere(T& container, const A& elementToAdd)
{
    bool canAdd = !contains(container, elementToAdd);

    if (canAdd)
        container.push_back(elementToAdd);

    return canAdd;
}

// Adds the element at the end only if doesn't already exist in the container
template <typename T, typename A>
bool addIfNotTherePointer(T& container, const A& elementToAdd)
{
    bool canAdd = !contains(container, elementToAdd);

    if (canAdd)
        container.push_back(elementToAdd);

    return canAdd;
}

template <typename T, typename A>
void copyInto(T& source, A& target)
{
    target.resize(source.size());
    std::ranges::copy(source, std::ranges::begin(target));
}

// Assigns the given value to every element of the container
template <typename Container, typename T>
void fill(Container& container, const T& value)
{
    for (auto& element: container)
        element = value;
}

// Assigns the given value to the first numItems elements of the container
template <typename Container, typename T>
void fill(Container& container, const T& value, int numItems)
{
    for (int index = 0; index < numItems; ++index)
        container[index] = value;
}

// Adds each element of the source to the corresponding element of the target
template <typename Target, typename Source>
void mixFrom(Target& target, Source& source)
{
    for (int index = 0; index < (int) target.size(); ++index)
        target[index] += source[index];
}

// Appends all elements of the source to the end of the target
template <typename Target, typename Source>
void addFrom(Target& target, const Source& source)
{
    if constexpr (requires { target.reserveAtLeast(0); })
        target.reserveAtLeast((int) target.size() + (int) source.size());

    for (auto& element: source)
        target.push_back(element);
}

// Appends the source elements at the given indexes to the end of the target
template <typename Target, typename Source>
void addFromIndexes(Target& target,
                    const Source& source,
                    std::initializer_list<int> indexes)
{
    for (auto index: indexes)
        target.push_back(source[index]);
}

// Replaces the target's contents with up to numItems elements of the source,
// starting at startIndex. The count is clamped to what the source contains
template <typename Target, typename Source>
void copyRange(Target& target, const Source& source, int startIndex, int numItems)
{
    auto numToCopy = std::min(numItems, (int) source.size() - startIndex);

    target.clear();

    if constexpr (requires { target.reserveAtLeast(0); })
        target.reserveAtLeast(numToCopy);

    for (int index = 0; index < numToCopy; ++index)
        target.push_back(source[startIndex + index]);
}

// Removes the half-open index range [startRange, endRange).
// Out-of-bounds ranges are ignored
template <typename Container>
void removeRange(Container& container, int startRange, int endRange)
{
    if (startRange < 0 || endRange > (int) container.size()
        || startRange >= endRange)
        return;

    if constexpr (requires { container.erase(container.begin(), container.end()); })
    {
        container.erase(container.begin() + startRange,
                        container.begin() + endRange);
    }
    else
    {
        for (int index = endRange - 1; index >= startRange; --index)
            removeAt(container, index);
    }
}

// Copies elements matching the predicate over the beginning of the target,
// which must already be large enough to hold them
template <typename Source, typename Target, typename Predicate>
void copyFilteredTo(const Source& source, Target& target, Predicate&& predicate)
{ std::copy_if(source.begin(), source.end(), target.begin(), predicate); }

// Appends the elements matching the predicate to the end of the target
template <typename Source, typename Target, typename Predicate>
void addFilteredTo(const Source& source, Target& target, Predicate&& predicate)
{
    std::copy_if(
        source.begin(), source.end(), std::back_inserter(target), predicate);
}

// Maps a valid index into the 0..1 range (0 = first element, 1 = last).
// Returns -1 for out-of-range indexes
template <typename FloatType, typename Container>
FloatType getIndexAsRelative(const Container& container, int index)
{
    auto size = (int) container.size();

    if (index < 0 || index >= size)
        return FloatType(-1);

    return Ranges::map(FloatType(index),
                       FloatType(0),
                       FloatType(size - 1),
                       FloatType(0),
                       FloatType(1));
}

// Maps a 0..1 proportion to an index into the container
template <typename Container, typename FloatType>
int getRelativeIndex(const Container& container, FloatType proportion)
{ return Ranges::getIndexProprtion(proportion, (int) container.size()); }

template <typename Container, typename FloatType>
auto& getRelative(Container& container, FloatType proportion)
{ return container[getRelativeIndex(container, proportion)]; }

// Returns the position of the first element equal to the given one as a
// 0..1 proportion, or -1 if it isn't found
template <typename FloatType, typename Container, typename A>
FloatType getRelativeIndexOf(const Container& container, const A& item)
{ return getIndexAsRelative<FloatType>(container, getIndexOf(container, item)); }

/**
 * Returns a fractional index representing the in-between position of a given
 * value. It is assumed the value is not in the container, and that the
 * container is sorted. Container_T must be forward iterable. Comparator_T is a
 * unary callable returning true if the given value is less than its argument.
 */
template <typename ContainerType, typename ComparatorType>
float getFractionalIndexOfValue(const ContainerType& container,
                                ComparatorType&& lessThan)
{
    float index = -0.5f;
    for (auto& element: container)
    {
        if (lessThan(element))
        {
            break;
        }
        index += 1.f;
    }
    return index;
}

/**
 *  Applies a callable over two containers, passing an element from each
 * container as arguments. Stops when the end of either container is reached.
 */
template <typename FirstContainerType,
          typename SecondContainerType,
          typename CallableType>
CallableType zipWith(const FirstContainerType& firstContainer,
                     const SecondContainerType& secondContainer,
                     CallableType&& call)
{
    auto first = firstContainer.begin();
    auto second = secondContainer.begin();
    for (; first != firstContainer.end() && second != secondContainer.end();
         ++first, ++second)
    {
        call(*first, *second);
    }
    return call;
}

/**
 *  Applies a callable over two containers, passing an element from each
 * container and the current index as arguments to the callable. Stops when the
 * end of either container is reached.
 */
template <typename FirstContainerType,
          typename SecondContainerType,
          typename CallableType>
CallableType zipWithIndexed(const FirstContainerType& firstContainer,
                            const SecondContainerType& secondContainer,
                            CallableType&& call)
{
    int index = 0;
    auto first = firstContainer.begin();
    auto second = secondContainer.begin();
    for (; first != firstContainer.end() && second != secondContainer.end();
         ++first, ++second, ++index)
    {
        call(*first, *second, index);
    }
    return call;
}

/**
 *  Applies the given function over each element of the source container and
 * returns the results in a new container of the same kind. The element type
 * may change; the result type comes from the container's Rebound alias.
 */
template <typename Container, typename Func>
auto transform(const Container& container, Func&& f)
{
    using NewElem = std::decay_t<decltype(f(*container.begin()))>;

    typename Container::template Rebound<NewElem> result;
    result.resize(container.size());

    std::ranges::transform(
        container, std::ranges::begin(result), std::forward<Func>(f));

    return result;
}

/**
 *  Returns a new container with all the elements satisfying the given
 * predicate.
 */
template <typename Container, typename Func>
auto filter(const Container& container, Func&& predicate)
{
    Container results;
    std::ranges::copy_if(
        container, std::back_inserter(results), std::forward<Func>(predicate));
    return results;
}

/**
 *  Performs a left fold on a container using the given function.
 */
template <typename ContainerType, typename Func>
auto fold(ContainerType&& container, Func func)
{
    assert(container.size() > 0);
    auto value = *container.begin();
    for (int i = 1; i < container.size(); ++i)
    {
        value = func(value, container[i]);
    }
    return value;
}

/**
 *  Performs a right fold on a container using the given function.
 */
template <typename ContainerType, typename Func>
auto foldr(ContainerType&& container, Func func)
{
    assert(container.size() > 0);
    auto value = *(container.end() - 1);
    for (int i = container.size() - 2; i >= 0; --i)
    {
        value = func(container[i], value);
    }
    return value;
}

template <typename Container, typename IndexType>
constexpr auto sizeType(IndexType index)
{ return static_cast<typename Container::size_type>(index); }

template <typename Container, typename IndexType>
auto& get(Container& container, IndexType index)
{ return container[sizeType<Container>(index)]; }

template <typename Container, typename IndexType>
const auto& get(const Container& container, IndexType index)
{ return container[sizeType<Container>(index)]; }

template <typename Container, typename SizeType, typename Factory>
void resizeTo(Container& vec, SizeType newSize, Factory factory)
{
    auto s = sizeType<Container>(newSize);

    if (vec.size() < s)
    {
        vec.reserve(s);

        while (vec.size() < s)
            vec.emplace_back(factory());
    }
    else if (vec.size() > s)
        vec.erase(vec.begin() + (long) s, vec.end());
}

namespace Detail
{
template <typename T>
struct isStdVectorHelper : std::false_type
{
};

template <typename T, typename Alloc>
struct isStdVectorHelper<std::vector<T, Alloc>> : std::true_type
{
};
} // namespace Detail

template <typename T>
constexpr bool isStdVector()
{ return Detail::isStdVectorHelper<T>::value; }
} // namespace EA::Vectors