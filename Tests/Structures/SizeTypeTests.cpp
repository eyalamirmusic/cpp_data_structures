#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/SizeType.h>
#include <ea_data_structures/Structures/Vector.h>

using namespace nano;

auto sizeTypeDefault = test("SizeType.default_is_zero") = []
{
    auto size = EA::SizeType();
    check(size.get<int>() == 0);
};

auto sizeTypeFromInt = test("SizeType.constructs_from_int") = []
{
    auto size = EA::SizeType(5);
    check(size.get<int>() == 5);
    check(size.get<size_t>() == size_t(5));
};

auto sizeTypeFromUnsignedInt = test("SizeType.constructs_from_unsigned_int") = []
{
    auto size = EA::SizeType(5u);
    check(size.get<unsigned int>() == 5u);
    check(size.get<int>() == 5);
};

auto sizeTypeFromOtherIntegers = test("SizeType.constructs_from_other_integers") = []
{
    check(EA::SizeType(short(3)).get<int>() == 3);
    check(EA::SizeType((unsigned short) 3).get<int>() == 3);
    check(EA::SizeType(3L).get<int>() == 3);
    check(EA::SizeType(3UL).get<int>() == 3);
    check(EA::SizeType(3LL).get<int>() == 3);
    check(EA::SizeType(3ULL).get<int>() == 3);
    check(EA::SizeType(size_t(3)).get<int>() == 3);
};

auto sizeTypeRejectsNonSizes = test("SizeType.rejects_non_size_types") = []
{
    static_assert(!std::is_constructible_v<EA::SizeType, bool>);
    static_assert(!std::is_constructible_v<EA::SizeType, float>);
    static_assert(!std::is_constructible_v<EA::SizeType, double>);
    static_assert(!std::is_constructible_v<EA::SizeType, int*>);
};

auto sizeTypeImplicitConversion = test("SizeType.implicit_conversion") = []
{
    auto size = EA::SizeType(7);

    int asInt = size;
    unsigned int asUnsigned = size;
    size_t asSizeT = size;
    long long asLongLong = size;

    check(asInt == 7);
    check(asUnsigned == 7u);
    check(asSizeT == size_t(7));
    check(asLongLong == 7LL);
};

auto sizeTypeNegativeRoundTrip = test("SizeType.negative_int_round_trips") = []
{
    auto size = EA::SizeType(-1);
    check(size.get<int>() == -1);
};

auto sizeTypeIndexesVector = test("SizeType.indexes_vector_with_any_integer") = []
{
    auto vec = EA::Vector<int> {10, 20, 30};

    check(vec[0] == 10);
    check(vec[1u] == 20);
    check(vec[size_t(2)] == 30);
    check(vec[short(0)] == 10);
    check(vec[2LL] == 30);
};

auto sizeTypeResizesVector = test("SizeType.resizes_vector_with_any_integer") = []
{
    auto vec = EA::Vector<int>();
    vec.resize(3u);
    check(vec.size() == 3);

    vec.resize(size_t(5), 42);
    check(vec.size() == 5);
    check(vec[4] == 42);
};
