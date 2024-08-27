#pragma once

//SizeType: A type meant to help connecting signed and unsigned containers
namespace EA
{
class SizeType
{
public:
    SizeType() = default;
    SizeType(size_t typeToUse)
        : size(typeToUse)
    {
    }

    SizeType(int typeToUse)
        : size((size_t) typeToUse)
    {
    }

    template <typename T>
    T get() const
    {
        return static_cast<T>(size);
    }

    operator int() const { return get<int>(); }
    operator size_t() const { return get<size_t>(); }

private:
    size_t size = 0;
};

} // namespace EA