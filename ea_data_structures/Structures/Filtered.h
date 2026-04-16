#pragma once

#include "Vector.h"

namespace EA::Utilities
{
//A filter-result view over a container: filter() records the indexes of
//elements matching a predicate; forEach() then visits just those elements,
//and removeAll() erases them from the underlying container. The container is
//referenced, not copied — it must outlive this object.
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