#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/BufferView.h>

using namespace nano;

auto bufferViewConstruct = test("BufferView.construct_and_size") = []
{
    int data[] = {1, 2, 3, 4};
    auto view = EA::BufferView<int>(data, 4);
    check(view.size() == 4);
};

auto bufferViewIndex = test("BufferView.indexing") = []
{
    int data[] = {10, 20, 30};
    auto view = EA::BufferView<int>(data, 3);
    check(view[0] == 10);
    check(view[2] == 30);
};

auto bufferViewIteration = test("BufferView.range_based_for") = []
{
    int data[] = {1, 2, 3};
    auto view = EA::BufferView<int>(data, 3);
    auto sum = 0;
    for (auto v: view)
        sum += v;
    check(sum == 6);
};

auto bufferViewWrite = test("BufferView.write_through_index") = []
{
    int data[] = {0, 0, 0};
    auto view = EA::BufferView<int>(data, 3);
    view[1] = 42;
    check(data[1] == 42);
};

auto twoDimBufferView = test("TwoDimensionalBufferView.iterates_channels") = []
{
    int chan0[] = {1, 2, 3};
    int chan1[] = {4, 5, 6};
    int* channels[] = {chan0, chan1};
    auto view = EA::getViewFor<int>(channels, 2, 3);

    auto channelSum = 0;
    for (auto chan: view)
        for (auto v: chan)
            channelSum += v;

    check(channelSum == 1 + 2 + 3 + 4 + 5 + 6);
};
