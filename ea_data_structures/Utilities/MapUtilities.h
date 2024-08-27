#pragma once

namespace EA::MapUtils
{
template <typename MapType, typename Key>
bool contains(const MapType& map, const Key& key)
{
    auto it = map.find(key);

    return it != map.end();
}

template <typename MapType, typename Key>
auto get(const MapType& map, const Key& key)
{
    auto it = map.find(key);

    if (it != map.end())
        return &it->second;

    return static_cast<decltype(&it->second)>(nullptr);
}

template <typename MapType, typename Key, typename DefaultValue>
auto get(const MapType& map, const Key& key, const DefaultValue& defaultVal)
{
    if (auto* val = MapUtils::get(map, key))
        return *val;
    else
        return decltype(*val)(defaultVal);
}

namespace Detail
{
template <typename T>
bool compare(const T& first, const T& second)
{
    return first == second;
}

template <typename KeyType, typename ValueType>
struct KeyValuePair
{
    template <typename... Args>
    explicit KeyValuePair(const KeyType& keyToUse, Args&&... args)
        : first(keyToUse)
        , second(std::forward<Args>(args)...)
    {
    }

    bool operator<(const KeyValuePair& other) const { return second < other.second; }

    bool keyEqualsTo(const KeyType& other) const
    {
        return Detail::compare(first, other);
    }

    const ValueType* operator->() const { return &second; }
    ValueType* operator->() { return &second; }
    ValueType& operator*() { return second; }
    const ValueType& operator*() const { return second; }

    KeyType first;
    ValueType second;
};

} // namespace Detail
} // namespace EA::MapUtils