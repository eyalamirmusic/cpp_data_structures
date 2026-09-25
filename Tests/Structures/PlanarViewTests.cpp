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

auto planarIsSmall = test("PlanarView.is_a_pointer_and_three_ints") = []
{
    //Three ints after a pointer round up to the pointer's alignment
    struct PointerAndThreeInts
    {
        void* pointer;
        int ints[3];
    };

    static_assert(sizeof(EA::PlanarView<float>) <= sizeof(PointerAndThreeInts));
    static_assert(std::is_trivially_copyable_v<EA::PlanarView<float>>);
};

auto planarStrided = test("PlanarView.strided_view_skips_the_gaps") = []
{
    //Two channels of five samples, viewed as two channels of three
    float data[] = {0, 1, 2, 3, 4, 10, 11, 12, 13, 14};
    auto view = EA::PlanarView<float>(data, 2, 3, 5);

    check(view.getNumChannels() == 2);
    check(view.getNumSamples() == 3);
    check(view.getChannelStride() == 5);
    check(view.getNumElements() == 6);
    check(view.data() == data);
    check(!view.isContiguous());

    check(view.getChannelPointer(0) == data);
    check(view.getChannelPointer(1) == data + 5);
    check(view[0].size() == 3);
    check(view[1].size() == 3);
    check(view[1][0] == 10.f);
    check(view[1][2] == 12.f);
};

auto planarStridedDeduction =
    test("PlanarView.deduces_element_type_with_a_stride") = []
{
    float data[] = {1, 2, 3, 4, 5, 6};

    auto view = EA::PlanarView(data, 2, 2, 3);
    static_assert(std::is_same_v<decltype(view), EA::PlanarView<float>>);

    check(view.getChannelStride() == 3);
};

auto planarSubView = test("PlanarView.sub_view_offsets_every_channel") = []
{
    float data[] = {0, 1, 2, 3, 10, 11, 12, 13};
    auto view = EA::PlanarView<float>(data, 2, 4);

    auto sub = view.subView(1, 2);

    check(sub.getNumChannels() == 2);
    check(sub.getNumSamples() == 2);
    check(sub.getChannelStride() == 4);
    check(!sub.isContiguous());
    check(sub.getChannelPointer(0) == data + 1);
    check(sub.getChannelPointer(1) == data + 5);
    check(sub[0][0] == 1.f);
    check(sub[0][1] == 2.f);
    check(sub[1][0] == 11.f);
    check(sub[1][1] == 12.f);
};

auto planarStridedIteration =
    test("PlanarView.iterating_a_strided_view_honours_the_stride") = []
{
    float data[] = {0, 1, 2, 3, 10, 11, 12, 13, 20, 21, 22, 23};
    auto view = EA::PlanarView<float>(data, 3, 4).subView(2);

    auto numChannelsSeen = 0;

    for (auto channel: view)
    {
        check(channel.size() == 2);
        check(channel.data() == data + numChannelsSeen * 4 + 2);
        check(channel[0] == (float) (numChannelsSeen * 10 + 2));
        ++numChannelsSeen;
    }

    check(numChannelsSeen == 3);
};

auto planarSubViewFill = test("PlanarView.fill_on_a_sub_view_leaves_gaps_alone") = []
{
    float data[] = {0, 1, 2, 3, 10, 11, 12, 13};

    EA::PlanarView<float>(data, 2, 4).subView(1, 2).fill(-1.f);

    float expected[] = {0, -1, -1, 3, 10, -1, -1, 13};

    for (auto index = 0; index < 8; ++index)
        check(data[index] == expected[index]);
};

auto planarConstConversionStride =
    test("PlanarView.const_conversion_keeps_the_stride") = []
{
    float data[] = {0, 1, 2, 3, 4, 10, 11, 12, 13, 14};
    auto view = EA::PlanarView<float>(data, 2, 3, 5);

    EA::PlanarView<const float> constView = view;

    check(constView.getChannelStride() == 5);
    check(constView.getChannelPointer(1) == data + 5);
    check(constView[1][0] == 10.f);
};

auto planarContiguity = test("PlanarView.is_contiguous_without_gaps") = []
{
    float data[] = {0, 1, 2, 3, 10, 11, 12, 13};
    auto view = EA::PlanarView<float>(data, 2, 4);

    check(EA::PlanarView<float>().isContiguous());
    check(view.isContiguous());
    check(EA::PlanarView<float>(EA::Span<float>(data), 2).isContiguous());
    check(EA::PlanarView<float>(data, 1, 2, 4).isContiguous());
    check(EA::PlanarView<float>(data, 1, 4).subView(1, 2).isContiguous());
    check(view.subView(0, 4).isContiguous());
    check(view.subView(0).isContiguous());
    check(!view.subView(0, 3).isContiguous());
};

auto planarSubViewClamps = test("PlanarView.sub_view_clamps_to_the_channel") = []
{
    float data[] = {0, 1, 2, 3, 10, 11, 12, 13};
    auto view = EA::PlanarView<float>(data, 2, 4);

    auto tail = view.subView(3, 10);
    check(tail.getNumSamples() == 1);
    check(tail[1][0] == 13.f);

    auto rest = view.subView(1);
    check(rest.getNumSamples() == 3);
    check(rest[1][2] == 13.f);

    auto pastTheEnd = view.subView(9, 2);
    check(pastTheEnd.getNumSamples() == 0);
    check(pastTheEnd.empty());

    auto atTheEnd = view.subView(4, 0);
    check(atTheEnd.empty());
    check(atTheEnd.getNumElements() == 0);
    check(atTheEnd.getChannel(0).empty());
};

auto planarEmptySubViewFlat =
    test("PlanarView.flat_on_an_empty_strided_sub_view") = []
{
    float data[] = {0, 1, 2, 3, 10, 11, 12, 13};
    auto empty = EA::PlanarView<float>(data, 2, 4).subView(2, 0);

    check(empty.getChannelStride() == 4);
    check(empty.isContiguous());
    check(empty.flat().empty());
};

auto planarConstexprSubView = test("PlanarView.sub_view_works_at_compile_time") = []
{
    static constexpr float data[] = {0, 1, 2, 3, 10, 11, 12, 13};
    constexpr auto view = EA::PlanarView<const float>(data, 2, 4);
    constexpr auto sub = view.subView(1, 2);

    static_assert(sub.getNumSamples() == 2);
    static_assert(sub.getChannelStride() == 4);
    static_assert(!sub.isContiguous());
    static_assert(sub[1][0] == 11.f);

    check(sub[0][1] == 2.f);
};
