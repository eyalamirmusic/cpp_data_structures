#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/Span.h>
#include <ea_data_structures/Structures/Vector.h>
#include <ea_data_structures/Structures/Array.h>
#include <ea_data_structures/Structures/StaticVector.h>
#include <ea_data_structures/Structures/SmallVector.h>
#include <ea_data_structures/Structures/FixedDynamicArray.h>
#include <array>
#include <vector>

using namespace nano;

namespace
{
struct Base
{
    int value = 0;
};

struct Derived : Base
{
    int extra = 0;
};

//Writing through a by-const-reference Span must still reach the source
void writeFirst(const EA::Span<int>& span, int value)
{
    span[0] = value;
}

int sumOf(EA::Span<const int> span)
{
    auto total = 0;

    for (auto value: span)
        total += value;

    return total;
}

//These have to be dependent on a template parameter: Clang hard-errors on a
//non-dependent requires-expression whose call doesn't resolve
template <typename SpanType>
constexpr bool canSort = requires(SpanType span) { span.sort(); };

template <typename SpanType>
constexpr bool canFill = requires(SpanType span) { span.fill(0); };

template <typename SpanType>
constexpr bool canReverse = requires(SpanType span) { span.reverse(); };

template <typename SpanType>
constexpr bool canQuery = requires(SpanType span) { span.contains(0); };
} // namespace

auto spanDefault = test("Span.default_is_empty") = []
{
    auto span = EA::Span<int>();

    check(span.size() == 0);
    check(span.empty());
    check(span.data() == nullptr);
    check(span.begin() == span.end());
};

auto spanFromPointerAndSize = test("Span.construct_from_pointer_and_size") = []
{
    int data[] = {1, 2, 3, 4};
    auto span = EA::Span<int>(data, 4);

    check(span.size() == 4);
    check(span.data() == data);
    check(!span.empty());
};

auto spanFromPointerPair = test("Span.construct_from_pointer_pair") = []
{
    int data[] = {1, 2, 3, 4};
    auto span = EA::Span<int>(data, data + 3);

    check(span.size() == 3);
    check(span.back() == 3);
};

auto spanFromCArray = test("Span.construct_from_c_array") = []
{
    int data[] = {5, 6, 7};
    auto span = EA::Span<int>(data);

    check(span.size() == 3);
    check(span.front() == 5);
    check(span.back() == 7);
};

auto spanFromVector = test("Span.construct_from_vector") = []
{
    auto vec = EA::Vector<int> {1, 2, 3};
    auto span = EA::Span<int>(vec);

    check(span.size() == 3);
    check(span.data() == vec.data());
};

auto spanFromArray = test("Span.construct_from_array") = []
{
    auto array = EA::Array {1, 2, 3, 4};
    auto span = EA::Span<int>(array);

    check(span.size() == 4);
    check(span.data() == array.data());
};

auto spanFromStaticVector = test("Span.construct_from_static_vector") = []
{
    auto vec = EA::StaticVector<int, 8> {1, 2, 3};
    auto span = EA::Span<int>(vec);

    check(span.size() == 3);
    check(span.data() == vec.data());
};

auto spanFromSmallVector = test("Span.construct_from_small_vector") = []
{
    auto vec = EA::SmallVector<int, 8> {1, 2, 3};
    auto span = EA::Span<int>(vec);

    check(span.size() == 3);
    check(span.data() == vec.data());
};

auto spanFromFixedDynamicArray = test("Span.construct_from_fixed_dynamic_array") = []
{
    auto array = EA::FixedDynamicArray<int>(3, 7);
    auto span = EA::Span<int>(array);

    check(span.size() == 3);
    check(span.data() == array.data());
    check(span.front() == 7);
    check(span.back() == 7);
};

auto spanFromStdContainers = test("Span.construct_from_std_containers") = []
{
    auto vec = std::vector<int> {1, 2, 3};
    auto array = std::array<int, 3> {4, 5, 6};

    check(EA::Span<int>(vec).size() == 3);
    check(EA::Span<int>(array).size() == 3);
    check(EA::Span<int>(array).front() == 4);
};

auto spanDeduction = test("Span.deduces_element_type") = []
{
    auto vec = EA::Vector<int> {1, 2, 3};
    auto span = EA::Span(vec);

    static_assert(std::is_same_v<decltype(span), EA::Span<int>>);
    check(span.size() == 3);

    const auto& constVec = vec;
    auto constSpan = EA::Span(constVec);

    static_assert(std::is_same_v<decltype(constSpan), EA::Span<const int>>);
    check(constSpan.size() == 3);
};

auto spanToConstConversion = test("Span.converts_to_span_of_const") = []
{
    auto vec = EA::Vector<int> {1, 2, 3};
    auto span = EA::Span<int>(vec);

    EA::Span<const int> constSpan = span;

    check(constSpan.size() == 3);
    check(constSpan[1] == 2);
    check(sumOf(span) == 6);

    static_assert(std::is_convertible_v<EA::Span<int>, EA::Span<const int>>);
    static_assert(!std::is_constructible_v<EA::Span<int>, EA::Span<const int>>);
};

auto spanConstCorrectness = test("Span.rejects_const_source_for_mutable_view") = []
{
    static_assert(std::is_constructible_v<EA::Span<int>, EA::Vector<int>&>);
    static_assert(!std::is_constructible_v<EA::Span<int>, const EA::Vector<int>&>);
    static_assert(
        std::is_constructible_v<EA::Span<const int>, const EA::Vector<int>&>);
};

auto spanRejectsSlicing = test("Span.rejects_derived_to_base") = []
{
    static_assert(std::is_constructible_v<EA::Span<Derived>, EA::Vector<Derived>&>);
    static_assert(!std::is_constructible_v<EA::Span<Base>, EA::Vector<Derived>&>);
};

auto spanRejectsTemporaries = test("Span.rejects_temporary_containers") = []
{
    static_assert(std::is_constructible_v<EA::Span<const int>, EA::Vector<int>&>);
    static_assert(!std::is_constructible_v<EA::Span<const int>, EA::Vector<int>&&>);
};

auto spanShallowConst = test("Span.const_reference_still_writes") = []
{
    auto vec = EA::Vector<int> {1, 2, 3};
    auto span = EA::Span<int>(vec);

    writeFirst(span, 42);

    check(vec[0] == 42);
};

auto spanWriteThrough = test("Span.writes_reach_the_source") = []
{
    auto vec = EA::Vector<int> {0, 0, 0};
    auto span = EA::Span<int>(vec);

    span[1] = 7;
    span.get(2) = 9;
    span.front() = 5;

    check(vec[0] == 5);
    check(vec[1] == 7);
    check(vec[2] == 9);
};

auto spanSizeInBytes = test("Span.size_in_bytes") = []
{
    int data[] = {1, 2, 3, 4};
    auto span = EA::Span<int>(data);

    check(span.sizeInBytes() == 4 * (int) sizeof(int));
};

auto spanIndexingWithAnyInteger = test("Span.indexes_with_any_integer") = []
{
    int data[] = {10, 20, 30};
    auto span = EA::Span<int>(data);

    check(span[0] == 10);
    check(span[1u] == 20);
    check(span[size_t(2)] == 30);
    check(span[short(0)] == 10);
    check(span[2LL] == 30);
};

auto spanIteration = test("Span.range_based_for") = []
{
    int data[] = {1, 2, 3};
    auto span = EA::Span<int>(data);
    auto sum = 0;

    for (auto value: span)
        sum += value;

    check(sum == 6);
};

auto spanReverseIteration = test("Span.reverse_iteration") = []
{
    int data[] = {1, 2, 3};
    auto span = EA::Span<int>(data);
    auto result = EA::Vector<int>();

    for (auto it = span.rbegin(); it != span.rend(); ++it)
        result.add(*it);

    check(result == EA::Vector<int> {3, 2, 1});
};

auto spanConstIterators = test("Span.const_iterators") = []
{
    int data[] = {1, 2, 3};
    auto span = EA::Span<int>(data);
    auto sum = 0;

    for (auto it = span.cbegin(); it != span.cend(); ++it)
        sum += *it;

    check(sum == 6);
};

auto spanFirstAndLast = test("Span.first_and_last") = []
{
    int data[] = {1, 2, 3, 4, 5};
    auto span = EA::Span<int>(data);

    auto firstTwo = span.first(2);
    check(firstTwo.size() == 2);
    check(firstTwo[0] == 1);
    check(firstTwo[1] == 2);

    auto lastTwo = span.last(2);
    check(lastTwo.size() == 2);
    check(lastTwo[0] == 4);
    check(lastTwo[1] == 5);
};

auto spanSubspan = test("Span.subspan") = []
{
    int data[] = {1, 2, 3, 4, 5};
    auto span = EA::Span<int>(data);

    auto middle = span.subspan(1, 3);
    check(middle.size() == 3);
    check(middle[0] == 2);
    check(middle[2] == 4);

    auto tail = span.subspan(2);
    check(tail.size() == 3);
    check(tail[0] == 3);
};

auto spanSubspanAliases = test("Span.subspan_aliases_the_source") = []
{
    auto vec = EA::Vector<int> {1, 2, 3, 4};
    auto span = EA::Span<int>(vec);

    span.subspan(1, 2).fill(0);

    check(vec == EA::Vector<int> {1, 0, 0, 4});
};

auto spanEquality = test("Span.equality") = []
{
    int first[] = {1, 2, 3};
    int second[] = {1, 2, 3};
    int third[] = {1, 2, 4};
    int shorter[] = {1, 2};

    auto firstSpan = EA::Span<int>(first);

    check(firstSpan == EA::Span<int>(second));
    check(firstSpan != EA::Span<int>(third));
    check(firstSpan != EA::Span<int>(shorter));
    check(firstSpan == EA::Span<const int>(second));
};

auto spanQueries = test("Span.query_helpers") = []
{
    int data[] = {10, 20, 30};
    auto span = EA::Span<int>(data);

    check(span.contains(20));
    check(!span.contains(40));
    check(span.getIndexOf(30) == 2);
    check(span.getIndexOf(40) == -1);
    check(span.getIndexIf([](auto value) { return value > 15; }) == 1);
    check(span.getLastElementIndex() == 2);
    check(*span.find(20) == 20);
    check(span.find(40) == nullptr);
};

auto spanFill = test("Span.fill") = []
{
    auto vec = EA::Vector<int> {1, 2, 3, 4};

    EA::Span<int>(vec).fill(9);
    check(vec == EA::Vector<int> {9, 9, 9, 9});

    EA::Span<int>(vec).fill(0, 2);
    check(vec == EA::Vector<int> {0, 0, 9, 9});
};

auto spanCopyFrom = test("Span.copy_from_stops_at_the_shorter_one") = []
{
    auto target = EA::Vector<int> {0, 0, 0};
    auto span = EA::Span<int>(target);

    span.copyFrom(EA::Vector<int> {1, 2, 3, 4, 5});
    check(target == EA::Vector<int> {1, 2, 3});

    auto shortTarget = EA::Vector<int> {0, 0, 0};
    EA::Span<int>(shortTarget).fillFrom(EA::Vector<int> {7, 8});
    check(shortTarget == EA::Vector<int> {7, 8, 0});
};

auto spanMixFrom = test("Span.mix_from") = []
{
    auto target = EA::Vector<int> {1, 2, 3};

    EA::Span<int>(target).mixFrom(EA::Vector<int> {10, 20, 30});

    check(target == EA::Vector<int> {11, 22, 33});
};

auto spanSort = test("Span.sort") = []
{
    auto vec = EA::Vector<int> {3, 1, 2};

    EA::Span<int>(vec).sort();
    check(vec == EA::Vector<int> {1, 2, 3});

    EA::Span<int>(vec).sort(false);
    check(vec == EA::Vector<int> {3, 2, 1});

    EA::Span<int>(vec).sort([](auto a, auto b) { return a < b; });
    check(vec == EA::Vector<int> {1, 2, 3});

    EA::Span<int>(vec).stableSort(false);
    check(vec == EA::Vector<int> {3, 2, 1});
};

auto spanSortsPartOfAContainer = test("Span.sorts_only_the_viewed_range") = []
{
    auto vec = EA::Vector<int> {9, 3, 1, 2, 8};

    EA::Span<int>(vec).subspan(1, 3).sort();

    check(vec == EA::Vector<int> {9, 1, 2, 3, 8});
};

auto spanReverse = test("Span.reverse") = []
{
    auto vec = EA::Vector<int> {1, 2, 3};

    EA::Span<int>(vec).reverse();

    check(vec == EA::Vector<int> {3, 2, 1});
};

auto spanRelative = test("Span.relative_helpers") = []
{
    int data[] = {10, 20, 30};
    auto span = EA::Span<int>(data);

    check(span.getRelative(0.f) == 10);
    check(span.getRelative(1.f) == 30);
    check(span.getRelativeIndex(1.f) == 2);
    check(span.getIndexAsRelative<float>(0) == 0.f);
    check(span.getIndexAsRelative<float>(2) == 1.f);
    check(span.getIndexAsRelative<float>(5) == -1.f);
};

auto spanConstHasNoMutators = test("Span.const_element_type_has_no_mutators") = []
{
    static_assert(canSort<EA::Span<int>>);
    static_assert(canFill<EA::Span<int>>);
    static_assert(canReverse<EA::Span<int>>);

    static_assert(!canSort<EA::Span<const int>>);
    static_assert(!canFill<EA::Span<const int>>);
    static_assert(!canReverse<EA::Span<const int>>);

    static_assert(canQuery<EA::Span<const int>>);
};

auto spanConstexpr = test("Span.works_at_compile_time") = []
{
    static constexpr int data[] = {1, 2, 3, 4};
    constexpr auto span = EA::Span<const int>(data);

    static_assert(span.size() == 4);
    static_assert(span[2] == 3);
    static_assert(span.front() == 1);
    static_assert(span.back() == 4);
    static_assert(span.subspan(1, 2).size() == 2);
    static_assert(span.subspan(1).front() == 2);
    static_assert(span.last(2).front() == 3);

    check(span.size() == 4);
};

auto spanIsSmall = test("Span.is_a_pointer_and_a_size") = []
{
    static_assert(sizeof(EA::Span<int>) <= sizeof(void*) + sizeof(int) * 2);
    static_assert(std::is_trivially_copyable_v<EA::Span<int>>);
};
