#pragma once

#include "Vector.h"

namespace EA::Utilities
{
template <typename ContainerType>
struct Filtered
{
    Filtered(ContainerType& containerToUse, int numItemsToReserve = 0)
        : container(containerToUse)
    {
        indexes.reserve(numItemsToReserve);
    }

    template <typename Pred>
    void filter(Pred&& pred)
    {
        indexes.clear();

        int index = 0;

        for (auto& element: container)
        {
            if (pred(element))
                indexes.add(index);

            ++index;
        }
    }

    template <typename Pred>
    void forEach(Pred&& pred)
    {
        for (auto index: indexes)
            pred(container[index]);
    }

    void removeAll()
    {
        for (int index = indexes.getLastElementIndex(); index >= 0; --index)
            container.removeAt(indexes[index]);

        indexes.clear();
    }

    ContainerType& container;
    Vector<int> indexes;
};

} // namespace EA::Utilities