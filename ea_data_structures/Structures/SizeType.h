#pragma once

#include <concepts>
#include <cstddef>

//SizeType: A type meant to help connecting signed and unsigned containers
namespace EA
{
//Any integer type that can describe a size or an index
template <typename T>
concept SizeCompatible = std::integral<T> && !std::same_as<T, bool>;

class SizeType
{
public:
    constexpr SizeType() = default;

    template <SizeCompatible T>
    constexpr SizeType(T typeToUse)
        : size(static_cast<size_t>(typeToUse))
    {
    }

    template <typename T>
    constexpr T get() const
    {
        return static_cast<T>(size);
    }

    template <SizeCompatible T>
    constexpr operator T() const
    {
        return get<T>();
    }

private:
    size_t size = 0;
};

} // namespace EA
