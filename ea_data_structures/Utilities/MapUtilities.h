#pragma once

namespace EA::MapUtils
{
template <typename MapType, typename Key>
bool contains(const MapType& map, Key& key)
{
    auto it = map.find(key);

    return it != map.end();
}

namespace Detail
{
template <typename T>
inline bool compare(const T& first, const T& second)
{
    return first == second;
}

template <typename KeyType, typename ValueType>
struct KeyValuePair
{
    explicit KeyValuePair(const KeyType& keyToUse,
                          const ValueType& valueToUse = ValueType())
        : first(keyToUse)
        , second(valueToUse)
    {
    }

    template <typename... Args>
    explicit KeyValuePair(const KeyType& keyToUse, Args&&... args)
        : first(keyToUse)
        , second(std::forward<Args>(args)...)
    {
    }

    bool keyEqualsTo(const KeyType& other) const
    {
        return Detail::compare(first, other);
    }

    KeyType first;
    ValueType second;
};

} // namespace Detail
} // namespace EA::MapUtils