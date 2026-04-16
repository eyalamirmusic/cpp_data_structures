#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/CircularBuffer.h>

using namespace nano;

auto circularSize = test("CircularBuffer.size") = []
{
    auto buffer = EA::CircularBuffer<int>(8);
    check(buffer.size() == 8);
};

auto circularWrapsPositive = test("CircularBuffer.wraps_positive_indexes") = []
{
    auto buffer = EA::CircularBuffer<int>(4, 0);
    buffer[0] = 10;
    buffer[1] = 20;
    buffer[2] = 30;
    buffer[3] = 40;
    check(buffer[4] == 10);
    check(buffer[5] == 20);
    check(buffer[9] == 20);
};

auto circularWrapsNegative = test("CircularBuffer.wraps_negative_indexes") = []
{
    auto buffer = EA::CircularBuffer<int>(4, 0);
    buffer[0] = 10;
    buffer[1] = 20;
    buffer[2] = 30;
    buffer[3] = 40;
    check(buffer[-1] == 40);
    check(buffer[-4] == 10);
    check(buffer[-5] == 40);
};

auto circularFill = test("CircularBuffer.fill") = []
{
    auto buffer = EA::CircularBuffer<int>(3);
    buffer.fill(42);
    check(buffer[0] == 42);
    check(buffer[1] == 42);
    check(buffer[2] == 42);
};

auto circularInitialValue = test("CircularBuffer.constructor_initial_value") = []
{
    auto buffer = EA::CircularBuffer<int>(3, 7);
    check(buffer[0] == 7);
    check(buffer[1] == 7);
    check(buffer[2] == 7);
};
