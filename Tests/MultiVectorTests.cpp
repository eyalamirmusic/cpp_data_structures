#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/MultiVector.h>

using namespace nano;

auto flatMultiVectorResize = test("FlatMultiVector.resize_sets_dimensions") = []
{
    auto m = EA::FlatMultiVector<int>();
    m.resize(3, 4);
    check(m.size() == 3);
};

auto flatMultiVectorIndex = test("FlatMultiVector.indexing_yields_view") = []
{
    auto m = EA::FlatMultiVector<int>();
    m.resize(2, 3);

    auto row0 = m[0];
    row0[0] = 1;
    row0[1] = 2;
    row0[2] = 3;

    auto row1 = m[1];
    row1[0] = 10;
    row1[1] = 20;
    row1[2] = 30;

    check(m[0][0] == 1);
    check(m[0][2] == 3);
    check(m[1][0] == 10);
    check(m[1][2] == 30);
};

auto flatMultiVectorDimensionStart =
    test("FlatMultiVector.getDimensionStart") = []
{
    auto m = EA::FlatMultiVector<int>();
    m.resize(4, 5);
    check(m.getDimensionStart(0) == 0);
    check(m.getDimensionStart(1) == 5);
    check(m.getDimensionStart(3) == 15);
};
