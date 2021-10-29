/*
created by: Eyal Amir
eyalamirmusic@gmail.com

These are simple templated vector/array operations meant for simplifying actions happen often.

Such as: Add only if item isn't there, remove if there, etc.

These will work for times where T is a simple type, or a type for which operator "=="
can be evaluated correctly.

To use, just include this header and call:
VectorUtilities::addIfNotThere(myVector, element).

 */

#pragma once

#include <algorithm>

namespace EA::Ranges
{
/** Remaps a value from a source range to a target range. */
template <typename Type>
Type map(Type sourceValue,
         Type sourceRangeMin,
         Type sourceRangeMax,
         Type targetRangeMin,
         Type targetRangeMax)
{
    return targetRangeMin
           + ((targetRangeMax - targetRangeMin) * (sourceValue - sourceRangeMin))
                 / (sourceRangeMax - sourceRangeMin);
}

/** Remaps a normalised value (between 0 and 1) to a target range.
    This effectively returns (targetRangeMin + value0To1 * (targetRangeMax - targetRangeMin)).
*/
template <typename Type>
constexpr Type map(Type value0To1, Type targetRangeMin, Type targetRangeMax)
{
    return map(value0To1, Type(0), Type(1), targetRangeMin, targetRangeMax);
}
} // namespace EA::Ranges

namespace EA::VectorUtilities
{
//Gets the index of an element that can be compared to each element of the container
//If it's not found, it will return -1;
template <typename T, typename A>
int getIndexOf(const T& container, const A& elementToCompare)
{
    int index = 0;

    for (auto& element: container)
    {
        if (element == elementToCompare)
            return index;

        ++index;
    }

    return -1;
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
{
    std::reverse(container.begin(), container.end());
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

//Check if an element that be compared to elements of this container exist.
template <typename T, typename A>
bool contains(const T& container, const A& elementToCheck)
{
    return getIndexOf(container, elementToCheck) >= 0;
}

//Gets a pointer to an element that can be compared to an element of this container,
//If the element doesn't exist, this returns a nullptr
template <typename T, typename A>
T* getElementPointer(const T& container, A elementToCompare)
{
    int index = getIndexOf(container, elementToCompare);

    if (index >= 0)
        return &container[index];
    else
        return nullptr;
}

//Gets a pointer to an element that can be compared to an element of this container
//This will crash if an element isn't found!
//
//If 'not finding' the element is a valid result, use getElementPointer() instead and check of nullptr
//Or check if the element is there first with contains(), which isn't as effecient.
template <typename T, typename A>
T& getElementRef(const T& container, A elementToCompare)
{
    return *getElementPointer(container, elementToCompare);
}

//Remove an element of the container at a certain index.
//If this is called in a loop that iterates over the container, make sure you do it in a reversed order
// (Starting from the end and going back to 0)
template <typename T>
void removeAt(T& container, int index)
{
    if (index < (int) container.size())
        container.erase(container.begin() + index);
}

template <typename Container, typename Callable>
bool eraseIf(Container& container, Callable callable)
{
    auto toRemove = std::remove_if(container.begin(), container.end(), callable);
    container.erase(toRemove, container.end());

    return toRemove != container.end();
}

//Removed the first match found in the container, going from beginning to end.
//If there are no elements found, it will do nothing
template <typename T, typename A>
void removeFirstMatch(T& container, A& elementToCheck)
{
    auto index = getIndexOf(container, elementToCheck);

    if (index >= 0)
        removeAt(container, index);
}

//Removes all matches of an element in the container
template <typename T, typename A>
void removeAllMatches(T& container, A& elementToCheck)
{
    auto numElements = (int) container.size();

    for (int index = numElements - 1; index >= 0; --index)
    {
        if (*(container.begin() + index) == elementToCheck)
            removeAt(container, index);
    }
}

//Adds the element at the end only if doesn't already exist in the container
template <typename T, typename A>
bool addIfNotThere(T& container, const A& elementToAdd)
{
    bool canAdd = !contains(container, elementToAdd);

    if (canAdd)
        container.push_back(elementToAdd);

    return canAdd;
}

//Adds the element at the end only if doesn't already exist in the container
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

    for (int index = 0; index < target.size(); ++index)
        target[index] = source[index];
}

/**
 * Returns a fractional index representing the in-between position of a given value.
 * It is assumed the value is not in the container, and that the container is sorted.
 * Container_T must be forward iterable.
 * Comparator_T is a unary callable returning true if the given value is less than its argument.
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
 *  Applies a callable over two containers, passing an element from each container as arguments.
 *  Stops when the end of either container is reached.
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
 *  Applies a callable over two containers, passing an element from each container and the current index as arguments to the callable.
 *  Stops when the end of either container is reached.
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
 *  Applies the given function over each element of the source container and returns the results in a new container.
 */
template <template <typename, int> typename VectorType,
          typename ElemType,
          int Sz,
          typename Func>
auto transform(const VectorType<ElemType, Sz>& container, Func&& f)
{
    using New_Elem_T = decltype(f(*container.begin()));

    VectorType<New_Elem_T, Sz> new_container;
    new_container.resize(container.size());

    std::transform(container.begin(),
                   container.end(),
                   new_container.begin(),
                   std::forward<Func>(f));

    return std::move(new_container);
}

/**
 *  Returns a new container with all the elements satisfying the given predicate.
 */
template <template <typename, int> typename VectorType,
          typename ElemType,
          int Sz,
          typename Func>
auto filter(const VectorType<ElemType, Sz>& container, Func predicate)
{
    VectorType<ElemType, Sz> results;

    for (auto&& elem: container)
        if (predicate(std::forward<decltype(elem)>(elem)))
            results.add(std::forward<decltype(elem)>(elem));

    return std::move(results);
}

/**
 *  Performs a left fold on a container using the given function.
 */
template <typename ContainerType, typename Func>
auto fold(ContainerType&& container, Func func)
{
    jassert(container.size() > 0);
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
    jassert(container.size() > 0);
    auto value = *(container.end() - 1);
    for (int i = container.size() - 2; i > 0; --i)
    {
        value = func(container[i], value);
    }
    return value;
}
} // namespace EA::VectorUtilities
