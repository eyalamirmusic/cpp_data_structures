#pragma once

#include "Vector.h"
#include <cassert>
#include "../Utilities/MapUtilities.h"

/*A std::map-like container that's implemented with an std::vector internally.

This class should provide a way slower access/inserting than std::map
but a dramatically faster iteration.

*/
namespace EA
{
template <typename KeyType, typename ValueType>
struct MapVector
{
    using key_type = KeyType;
    using ElementType = MapUtils::Detail::KeyValuePair<KeyType, ValueType>;
    using ContainerType = Vector<ElementType>;
    using Iterator = typename ContainerType::Iterator;
    using ConstIterator = typename ContainerType::Const_Iterator;

    Iterator begin() { return container.begin(); }
    Iterator end() { return container.end(); }

    ConstIterator begin() const { return container.begin(); }
    ConstIterator end() const { return container.end(); }

    ConstIterator find(const KeyType& key) const
    {
        auto index = getIndexOf(key);

        if (index >= 0)
            return {begin() + index};

        return end();
    }

    template <typename T>
    const ValueType* getFirstMatch(const T& other) const
    {
        for (auto& element: container)
        {
            if (element.second == other)
                return &element.second;
        }

        return nullptr;
    }

    template <typename T>
    bool hasMatch(const T& other) const
    {
        return getFirstMatch(other) != nullptr;
    }

    const ValueType* getValue(const KeyType& key) const
    {
        auto index = getIndexOf(key);

        if (index >= 0)
            return &container[index].second;

        return nullptr;
    }

    ValueType* getValue(const KeyType& key)
    {
        auto index = getIndexOf(key);

        if (index >= 0)
            return &container[index].second;

        return nullptr;
    }

    void remove(const KeyType& key)
    {
        auto index = getIndexOf(key);

        if (index >= 0)
            removeAt(index);
    }

    void removeAt(int index) { container.removeAt(index); }

    template <typename Callable>
    void eraseIf(Callable&& callable)
    {
        auto eraseFunc = [callable](const auto& pair)
        { return callable(pair.second); };

        container.eraseIf(eraseFunc);
    }

    ValueType& getOrCreate(const KeyType& key)
    {
        if (auto* value = getValue(key))
            return *value;

        return container.create(key).second;
    }

    ValueType& operator[](const KeyType& key) { return getOrCreate(key); }

    void clear() { container.clear(); }
    void reserve(int numItems) { container.reserve(numItems); }

    template <typename... Args>
    ValueType& emplace(const KeyType& key, Args&&... args)
    {
        return container.create(key, std::forward<Args>(args)...).second;
    }

    ValueType& get(int index) { return container[index].second; }

    const KeyType& getKey(int index) { return container[index].first; }

    template <typename Func>
    const KeyType* getKeyBy(Func comparison) const
    {
        for (auto& element: container)
        {
            if (comparison(element.second))
                return &element.first;
        }

        return nullptr;
    }

    const KeyType* getKeyByValue(const ValueType& value) const
    {
        auto comparison = [&](const ValueType& v) { return value == v; };
        return getKeyBy(comparison);
    }

    ValueType& back() { return container.back().second; }
    const ValueType& back() const { return container.back().second; }

    const KeyType& getKey(const ValueType& value)
    {
        if (auto* key = getKeyByValue(value))
            return *key;

        assert(false); //Invalid value

        static KeyType invalidKey;
        return invalidKey;
    }

    ElementType& getPair(int index) { return container[index]; }

    int size() const { return container.size(); }

    void sortByKey(bool forward = true)
    {
        auto pred = [](const auto& first, const auto& second)
        { return first.first < second.first; };

        container.sort(pred, forward);
    }

    void sortByValue(bool reverse = false) { container.sort(reverse); }

    bool empty() const { return container.empty(); }

    int getIndexOf(const KeyType& key) const
    {
        int index = 0;

        for (auto& element: container)
        {
            if (element.keyEqualsTo(key))
                return index;

            ++index;
        }

        return -1;
    }

    ContainerType container;
};

} // namespace EA