#pragma once

//OwnedVector: A class representing a vector of OwningPointers
//(a replacement for std::unique_ptr)
// with some helper functions for this particlar use case
#include "../Pointers/OwningPointer.h"
#include "Vector.h"

namespace EA
{
template <typename T>
class OwnedVector : public Vector<OwningPointer<T>>
{
public:
    using Vector<OwningPointer<T>>::Vector;

    using ValueType = OwningPointer<T>;

    //Finds the index in the container by comparing a raw pointer with the address of the owned
    //object
    template <typename A>
    int getIndexOfItem(A* item)
    {
        int index = 0;

        for (auto& element: this->container)
        {
            if (element.get() == item)
                return index;

            ++index;
        }

        return -1;
    }

    void removeItem(T& item)
    {
        const auto idx = getIndexOfItem(&item);
        if (idx > -1)
            this->removeAt(idx);
    }

    //Finds the index of the element by using the == operator of the owned object
    //with the element passed in
    template <typename ObjectType>
    int getIndexOfElement(const ObjectType& elementToFind)
    {
        int index = 0;

        for (auto& element: this->container)
        {
            if (*element == elementToFind)
                return index;

            ++index;
        }

        return -1;
    }

    template <typename ObjectType>
    bool contains(const ObjectType& elementToCheck)
    {
        return getIndexOfElement(elementToCheck) >= 0;
    }

    template <typename ObjectType>
    bool addElementIfNotThere(const ObjectType& elementToAdd)
    {
        bool canAdd = !contains(elementToAdd);

        if (canAdd)
            createNew(elementToAdd);

        return canAdd;
    }

    template <typename ObjectType = T, typename... Args>
    ObjectType& insertNew(int position, Args&&... args)
    {
        auto& created = this->insertAt(position);
        return *created.template create<ObjectType>(std::forward<Args>(args)...);
    }

    template <typename ObjectType = T, typename... Args>
    void insertNewRange(int start, int numItems, Args&&... args)
    {
        while (numItems > 0)
        {
            insertNew<ObjectType>(start, std::forward<Args>(args)...);
            --numItems;
        }
    }

    template <typename... Args>
    T& createNew(Args&&... args)
    {
        auto newItem = new T(std::forward<Args>(args)...);

        return *this->create(newItem);
    }

    template <typename A>
    void removeAllMatches(const A& elementToCheck)
    {
        for (int index = this->getLastElementIndex(); index >= 0; --index)
        {
            if (*this->get(index) == elementToCheck)
                this->removeAt(index);
        }
    }

    template <typename Derived, typename... Args>
    Derived& createDerived(Args&&... args)
    {
        auto newElement = new Derived(std::forward<Args>(args)...);

        auto convertedElement = static_cast<T*>(newElement);
        this->getVector().emplace_back(convertedElement);

        return *newElement;
    }
};
} // namespace EA
