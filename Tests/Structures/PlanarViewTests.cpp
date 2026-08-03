#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/PlanarView.h>
#include <ea_data_structures/Structures/Vector.h>

using namespace nano;

namespace
{
//Two channels of three samples, planar: channel 0 is {1, 2, 3}
float sharedStorage[] = {1, 2, 3, 4, 5, 6};

//Returns the view by value, so the caller only ever sees a temporary
EA::PlanarView<float> makeSharedView()
{
    return {sharedStorage, 2, 3};
}

float sumOf(EA::PlanarView<const float> view)
{
    auto total = 0.f;

    for (auto channel: view)
    {
        for (auto sample: channel)
            total += sample;
    }

    return total;
}

template <typename ViewType>
constexpr bool canFill = requires(ViewType view) { view.fill(0.f); };
} // namespace

auto planarDefault = test("PlanarView.default_is_empty") = []
{
    auto view = EA::PlanarView<float>();

    check(view.empty());
    check(view.getNumChannels() == 0);
    check(view.getNumSamples() == 0);
    check(view.getNumElements() == 0);
    check(view.data() == nullptr);
    check(view.begin() == view.end());
};

auto planarFromPointer = test("PlanarView.construct_from_pointer_and_shape") = []
{
    float data[] = {1, 2, 3, 4, 5, 6};
    auto view = EA::PlanarView<float>(data, 2, 3);

    check(!view.empty());
    check(view.getNumChannels() == 2);
    check(view.getNumSamples() == 3);
    check(view.getNumElements() == 6);
    check(view.data() == data);
};

auto planarFromFlatSpan = test("PlanarView.splits_a_flat_span_evenly") = []
{
    auto vec = EA::Vector<float> {1, 2, 3, 4, 5, 6, 7, 8};
    auto view = EA::PlanarView<float>(EA::Span<float>(vec), 4);

    check(view.getNumChannels() == 4);
    check(view.getNumSamples() == 2);
    check(view[0][0] == 1.f);
    check(view[3][1] == 8.f);
};

auto planarChannelOffsets =
    test("PlanarView.slices_channels_at_the_right_offset") = []
{
    float data[] = {1, 2, 3, 4, 5, 6};
    auto view = EA::PlanarView<float>(data, 2, 3);

    auto first = view.getChannel(0);
    check(first.size() == 3);
    check(first[0] == 1.f);
    check(first[2] == 3.f);

    auto second = view[1];
    check(second.size() == 3);
    check(second[0] == 4.f);
    check(second[2] == 6.f);

    check(view.getChannelPointer(0) == data);
    check(view.getChannelPointer(1) == data + 3);
};

auto planarWriteThrough = test("PlanarView.writes_reach_the_source") = []
{
    float data[] = {0, 0, 0, 0};
    auto view = EA::PlanarView<float>(data, 2, 2);

    view[0][1] = 7.f;
    view[1].fill(9.f);

    check(data[1] == 7.f);
    check(data[2] == 9.f);
    check(data[3] == 9.f);
};

auto planarIteration = test("PlanarView.iterates_one_span_per_channel") = []
{
    float data[] = {1, 2, 3, 4, 5, 6};
    auto view = EA::PlanarView<float>(data, 2, 3);

    auto numChannelsSeen = 0;
    auto total = 0.f;

    for (auto channel: view)
    {
        ++numChannelsSeen;
        check(channel.size() == 3);

        for (auto sample: channel)
            total += sample;
    }

    check(numChannelsSeen == 2);
    check(total == 21.f);
};

//The reason the iterator holds the shape by value instead of pointing back at
//the view it came from
auto planarIteratorIsSelfContained =
    test("PlanarView.iterator_outlives_the_view") = []
{
    float data[] = {1, 2, 3, 4, 5, 6};

    //The view is a temporary and is gone by the next statement
    auto it = EA::PlanarView<float>(data, 2, 3).begin();

    check((*it).size() == 3);
    check((*it)[0] == 1.f);

    ++it;
    check((*it)[0] == 4.f);

    --it;
    check((*it)[0] == 1.f);
};

auto planarIteratesTemporary = test("PlanarView.iterates_a_temporary_view") = []
{
    auto total = 0.f;

    for (auto channel: makeSharedView())
    {
        for (auto sample: channel)
            total += sample;
    }

    check(total == 21.f);
};

auto planarFlat = test("PlanarView.flat_returns_the_whole_block") = []
{
    float data[] = {1, 2, 3, 4, 5, 6};
    auto view = EA::PlanarView<float>(data, 2, 3);

    auto flat = view.flat();

    check(flat.size() == 6);
    check(flat.data() == data);
    check(flat[5] == 6.f);
};

auto planarFill = test("PlanarView.fill_covers_every_channel") = []
{
    float data[] = {1, 2, 3, 4, 5, 6};

    EA::PlanarView<float>(data, 2, 3).fill(0.f);

    for (auto sample: data)
        check(sample == 0.f);
};

auto planarToConstConversion = test("PlanarView.converts_to_view_of_const") = []
{
    float data[] = {1, 2, 3, 4, 5, 6};
    auto view = EA::PlanarView<float>(data, 2, 3);

    EA::PlanarView<const float> constView = view;

    check(constView.getNumChannels() == 2);
    check(constView[1][0] == 4.f);
    check(sumOf(view) == 21.f);

    static_assert(
        std::is_convertible_v<EA::PlanarView<float>, EA::PlanarView<const float>>);
    static_assert(!std::is_constructible_v<EA::PlanarView<float>,
                                           EA::PlanarView<const float>>);
};

auto planarConstHasNoMutators =
    test("PlanarView.const_element_type_has_no_fill") = []
{
    static_assert(canFill<EA::PlanarView<float>>);
    static_assert(!canFill<EA::PlanarView<const float>>);
};

auto planarDeduction = test("PlanarView.deduces_element_type") = []
{
    float data[] = {1, 2, 3, 4};

    auto fromPointer = EA::PlanarView(data, 2, 2);
    static_assert(std::is_same_v<decltype(fromPointer), EA::PlanarView<float>>);

    auto fromSpan = EA::PlanarView(EA::Span<float>(data), 2);
    static_assert(std::is_same_v<decltype(fromSpan), EA::PlanarView<float>>);

    check(fromPointer.getNumSamples() == 2);
    check(fromSpan.getNumSamples() == 2);
};

auto planarZeroChannels = test("PlanarView.zero_channels_is_safe") = []
{
    float data[] = {1, 2, 3};

    auto noChannels = EA::PlanarView<float>(data, 0, 3);
    check(noChannels.empty());
    check(noChannels.getNumElements() == 0);
    check(noChannels.getChannel(0).empty());
    check(noChannels.flat().empty());
    check(noChannels.begin() == noChannels.end());

    auto noSamples = EA::PlanarView<float>(data, 2, 0);
    check(noSamples.empty());
    check(noSamples.getNumElements() == 0);
    check(noSamples.getChannel(1).empty());

    auto emptyFlat = EA::PlanarView<float>(EA::Span<float>(), 2);
    check(emptyFlat.empty());
    check(emptyFlat.getNumSamples() == 0);
};

auto planarConstexpr = test("PlanarView.works_at_compile_time") = []
{
    static constexpr float data[] = {1, 2, 3, 4, 5, 6};
    constexpr auto view = EA::PlanarView<const float>(data, 2, 3);

    static_assert(view.getNumChannels() == 2);
    static_assert(view.getNumSamples() == 3);
    static_assert(view.getNumElements() == 6);
    static_assert(view[1][0] == 4.f);
    static_assert(view.flat().size() == 6);

    check(view.getNumChannels() == 2);
};

auto planarIsSmall = test("PlanarView.is_a_pointer_and_two_ints") = []
{
    static_assert(sizeof(EA::PlanarView<float>) <= sizeof(void*) + sizeof(int) * 2);
    static_assert(std::is_trivially_copyable_v<EA::PlanarView<float>>);
};
