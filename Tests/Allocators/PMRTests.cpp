#include <Helpers/AllocationHelpers.h>
#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Allocators/MultiPoolAllocator.h>
#include <ea_data_structures/Allocators/PMR.h>

#include <algorithm>
#include <memory>
#include <memory_resource>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace nano;
using EA::TestHelpers::countGlobalHeapCalls;
using EA::TestHelpers::proxyAllocationsPerContainer;
using EA::TestHelpers::StackResource;

namespace
{
//A PMR-aware type whose constructor takes the resource first, which is the
//shape PMR::Object forwards its extra arguments to.
struct FilledBuffer
{
    FilledBuffer(std::pmr::memory_resource* resource, std::size_t size, int fill)
        : values(size, fill, resource)
    {
    }

    std::pmr::vector<int> values;
};

//Long enough to defeat the small-string optimisation on every standard library,
//so each string really allocates.
constexpr auto longText = "a string that is far too long for any SSO buffer";
} // namespace

//PMR::Object

auto objectStoresResource =
    test("PMR.Object.constructed_from_reference_or_pointer") = []
{
    auto stack = StackResource<1024>();
    auto& resource = stack.counter;

    auto fromReference = EA::PMR::Object<std::pmr::vector<int>>(resource);
    auto fromPointer = EA::PMR::Object<std::pmr::vector<int>>(&resource);

    check(fromReference.getResource() == &resource);
    check(fromPointer.getResource() == &resource);
    check(fromReference->get_allocator().resource() == &resource);
    check(fromPointer->get_allocator().resource() == &resource);
};

auto objectAccessors =
    test("PMR.Object.get_arrow_and_star_reach_the_same_object") = []
{
    auto stack = StackResource<1024>();
    auto object = EA::PMR::Object<std::pmr::vector<int>>(stack.counter);

    object->push_back(1);
    (*object).push_back(2);
    object.get().push_back(3);

    const auto& constObject = object;

    check(&constObject.get() == &object.get());
    check(&*constObject == &object.get());
    check(constObject->size() == 3);
    check(constObject.get()[0] == 1);
    check(constObject.get()[2] == 3);
};

auto objectForwardsArgs = test("PMR.Object.forwards_args_after_the_resource") = []
{
    auto stack = StackResource<1024>();

    auto object = EA::PMR::Object<FilledBuffer>(stack.counter, 8, 7);

    check(object->values.size() == 8);
    check(object->values[0] == 7);
    check(object->values[7] == 7);
    check(object->values.get_allocator().resource() == &stack.counter);
    check(stack.counter.allocations == 1 + proxyAllocationsPerContainer);
};

auto objectNoGlobalHeap =
    test("PMR.Object.allocates_through_its_resource_not_global_new") = []
{
    auto stack = StackResource<1024>();

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            auto object = EA::PMR::Object<FilledBuffer>(stack.counter, 32, 1);
            object->values.push_back(2);
        });

    check(heapCalls == 0);
    check(stack.counter.allocations >= 1);
    check(stack.counter.live() == 0);
};

auto objectCopyKeepsResource = test("PMR.Object.copy_uses_the_source_resource") = []
{
    auto stack = StackResource<4096>();
    auto source = EA::PMR::Vector<int>(stack.counter);

    for (auto index = 0; index < 16; ++index)
        source->push_back(index);

    auto allocationsBefore = stack.counter.allocations;
    auto copySize = std::size_t {0};
    auto sameContents = false;
    auto copyResource = static_cast<std::pmr::memory_resource*>(nullptr);
    auto copyAllocatorResource = static_cast<std::pmr::memory_resource*>(nullptr);

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            auto copy = source;
            copySize = copy->size();
            sameContents = *copy == *source;
            copyResource = copy.getResource();
            copyAllocatorResource = copy->get_allocator().resource();
        });

    check(heapCalls == 0);
    check(copySize == 16);
    check(sameContents);
    check(copyResource == &stack.counter);
    check(copyAllocatorResource == &stack.counter);
    check(stack.counter.allocations
          == allocationsBefore + 1 + proxyAllocationsPerContainer);
};

auto objectMoveKeepsResource =
    test("PMR.Object.move_construction_keeps_resource") = []
{
    auto stack = StackResource<1024>();
    auto source = EA::PMR::Vector<int>(stack.counter);
    source->assign({1, 2, 3});

    const auto* buffer = source->data();
    auto allocationsBefore = stack.counter.allocations;
    auto stoleBuffer = false;
    auto sameContents = false;
    auto movedResource = static_cast<std::pmr::memory_resource*>(nullptr);
    auto movedAllocatorResource = static_cast<std::pmr::memory_resource*>(nullptr);

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            auto moved = std::move(source);
            stoleBuffer = moved->data() == buffer;
            sameContents = *moved == std::pmr::vector<int>({1, 2, 3}, &stack.arena);
            movedResource = moved.getResource();
            movedAllocatorResource = moved->get_allocator().resource();
        });

    //The move takes over the buffer; only MSVC's debug STL allocates anything,
    //a fresh container proxy for the moved-to vector.
    check(heapCalls == 0);
    check(stack.counter.allocations
          == allocationsBefore + proxyAllocationsPerContainer);
    check(stoleBuffer);
    check(sameContents);
    check(movedResource == &stack.counter);
    check(movedAllocatorResource == &stack.counter);
};

auto objectAssignSameResource =
    test("PMR.Object.copy_assignment_with_shared_resource") = []
{
    auto stack = StackResource<4096>();
    auto source = EA::PMR::Vector<int>(stack.counter);
    auto target = EA::PMR::Vector<int>(stack.counter);
    source->assign({4, 5, 6});
    target->assign({9});

    auto heapCalls = countGlobalHeapCalls([&] { target = source; });

    check(heapCalls == 0);
    check(*target == *source);
    check(target.getResource() == &stack.counter);
    check(target->get_allocator().resource() == &stack.counter);
};

//std::pmr containers never propagate their allocator on assignment, so neither
//does Object: across different resources, the target keeps its own resource
//and the contents are copied (or moved element-wise) into it.
auto objectAssignAcrossResources =
    test("PMR.Object.copy_assignment_keeps_resource_in_sync") = []
{
    auto first = StackResource<1024>();
    auto second = StackResource<1024>();
    auto copyTarget = EA::PMR::Vector<int>(first.counter);
    auto moveTarget = EA::PMR::Vector<int>(first.counter);
    auto source = EA::PMR::Vector<int>(second.counter);
    auto moveSource = EA::PMR::Vector<int>(second.counter);
    source->assign({1, 2, 3});
    moveSource->assign({4, 5});
    auto secondAllocationsBefore = second.counter.allocations;

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            copyTarget = source;
            moveTarget = std::move(moveSource);
        });

    check(heapCalls == 0);

    check(*copyTarget == *source);
    check(copyTarget.getResource() == &first.counter);
    check(copyTarget->get_allocator().resource() == &first.counter);

    check(moveTarget->size() == 2);
    check(moveTarget.get()[0] == 4);
    check(moveTarget.get()[1] == 5);
    check(moveTarget.getResource() == &first.counter);
    check(moveTarget->get_allocator().resource() == &first.counter);

    //Both targets allocated from their own resource, never from the source's
    //(plus a debug-STL proxy each when they were constructed).
    check(first.counter.allocations == 2 + 2 * proxyAllocationsPerContainer);
    check(second.counter.allocations == secondAllocationsBefore);
};

//PMR::Vector

auto vectorAlias = test("PMR.Vector.is_an_object_wrapping_pmr_vector") = []
{
    using IntVector = EA::PMR::Vector<int>;

    static_assert(std::is_same_v<IntVector, EA::PMR::Object<std::pmr::vector<int>>>);
    static_assert(std::is_same_v<IntVector::Type, std::pmr::vector<int>>);
};

auto vectorNoGlobalHeap =
    test("PMR.Vector.growth_allocates_through_resource_not_global_new") = []
{
    auto stack = StackResource<8192>();
    auto sum = 0;

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            auto values = EA::PMR::Vector<int>(stack.counter);

            for (auto index = 0; index < 100; ++index)
                values->push_back(index);

            for (auto value: *values)
                sum += value;
        });

    check(heapCalls == 0);
    check(sum == 99 * 100 / 2);
    check(stack.counter.allocations > 1);
    check(stack.counter.live() == 0);
};

auto vectorPropagatesToElements =
    test("PMR.Vector.pmr_elements_share_the_vector_resource") = []
{
    auto stack = StackResource<4096>();
    auto elementResource = static_cast<std::pmr::memory_resource*>(nullptr);
    auto textIntact = false;

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            auto strings = EA::PMR::Vector<std::pmr::string>(stack.counter);
            strings->reserve(2);
            strings->emplace_back(longText);
            strings->emplace_back(longText);
            elementResource = strings->back().get_allocator().resource();
            textIntact = strings->front() == longText;
        });

    check(heapCalls == 0);
    check(textIntact);
    check(elementResource == &stack.counter);
    //One buffer for the vector, one per string (and a debug-STL proxy each).
    check(stack.counter.allocations == 3 + 3 * proxyAllocationsPerContainer);
    check(stack.counter.live() == 0);
};

auto vectorWithMemoryPool =
    test("PMR.Vector.on_a_memory_pool_resource_skips_heap") = []
{
    auto pool = EA::Allocators::MemoryPoolResource(4096, 16, 64);
    auto size = std::size_t {0};

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            auto values = EA::PMR::Vector<double>(pool);

            for (auto index = 0; index < 256; ++index)
                values->push_back(index * 0.5);

            auto copy = values;
            size = copy->size();
        });

    check(heapCalls == 0);
    check(size == 256);
};

//PMR::makeShared

auto makeSharedThroughResource =
    test("PMR.makeShared.allocates_and_frees_through_resource") = []
{
    auto stack = StackResource<1024>();
    auto first = 0;
    auto second = 0.0;
    auto allocationsWhileAlive = 0;

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            auto shared =
                EA::PMR::makeShared<std::pair<int, double>>(stack.counter, 3, 2.5);
            first = shared->first;
            second = shared->second;
            allocationsWhileAlive = stack.counter.allocations;
        });

    check(heapCalls == 0);
    check(first == 3);
    check(second == 2.5);
    //Object and control block share a single allocation.
    check(allocationsWhileAlive == 1);
    check(stack.counter.deallocations == 1);
};

auto makeSharedOnPool =
    test("PMR.makeShared.released_block_is_reused_by_the_pool") = []
{
    auto pool = EA::Allocators::MemoryPoolResource(256, 4, 16);
    const void* firstAddress = nullptr;
    const void* secondAddress = nullptr;

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            auto shared = EA::PMR::makeShared<int>(pool, 42);
            firstAddress = shared.get();
            shared.reset();

            auto again = EA::PMR::makeShared<int>(pool, 43);
            secondAddress = again.get();
        });

    check(heapCalls == 0);
    check(firstAddress == secondAddress);
};

//PMR::makeOwned

namespace
{
//Counts its own destructions through a pointer the test owns.
struct Tracked
{
    Tracked(int valueToUse, int& destructionsToUse)
        : value(valueToUse)
        , destructions(&destructionsToUse)
    {
    }

    ~Tracked() { ++*destructions; }

    int value;
    int* destructions;
};

struct ThrowsOnConstruction
{
    ThrowsOnConstruction() { throw std::runtime_error("constructor failed"); }
};

struct Voice
{
    double phase = 0.0;
    float gain = 1.f;
    int note = 0;
};

struct Base
{
    Base(int& destructionsToUse)
        : destructions(&destructionsToUse)
    {
    }

    virtual ~Base() { ++*destructions; }

    int* destructions;
};

//A polymorphic Mixin listed first becomes the primary base, so the Base
//subobject isn't at the start of the allocation.
struct Mixin
{
    virtual ~Mixin() = default;

    long long padding[3] {};
};

//The over-alignment (and the padding it brings) is the point of this type, so
//silence MSVC's C4324 "structure was padded due to alignment specifier".
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4324)
#endif

struct alignas(32) Derived
    : Mixin
    , Base
{
    Derived(int& destructionsToUse, int valueToUse)
        : Base(destructionsToUse)
        , value(valueToUse)
    {
    }

    ~Derived() override { ++*destructions; }

    int value;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

struct PlainBase
{
    int value = 0;
};

struct PlainDerived : PlainBase
{
    int extra = 0;
};
} // namespace

auto makeOwnedThroughResource =
    test("PMR.makeOwned.allocates_constructs_and_frees_through_resource") = []
{
    auto stack = StackResource<1024>();
    auto destructions = 0;
    auto value = 0;
    auto allocationsWhileAlive = 0;
    auto bytesWhileAlive = std::size_t {0};

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            auto owned =
                EA::PMR::makeOwned<Tracked>(stack.counter, 42, destructions);
            value = owned->value;
            allocationsWhileAlive = stack.counter.allocations;
            bytesWhileAlive = stack.counter.bytesAllocated;
        });

    check(heapCalls == 0);
    check(value == 42);
    check(allocationsWhileAlive == 1);
    check(bytesWhileAlive == sizeof(Tracked));
    check(destructions == 1);
    check(stack.counter.live() == 0);
    check(stack.counter.bytesDeallocated == sizeof(Tracked));
    check(stack.counter.lastDeallocation.pointer
          == stack.counter.lastAllocation.pointer);
    check(stack.counter.lastDeallocation.alignment == alignof(Tracked));
};

auto makeOwnedUsesAllocator =
    test("PMR.makeOwned.pmr_aware_object_gets_the_same_resource") = []
{
    auto stack = StackResource<1024>();
    auto innerResource = static_cast<std::pmr::memory_resource*>(nullptr);

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            auto owned = EA::PMR::makeOwned<std::pmr::vector<int>>(stack.counter);
            owned->assign({1, 2, 3});
            innerResource = owned->get_allocator().resource();
        });

    check(heapCalls == 0);
    check(innerResource == &stack.counter);
    //The Owned allocation, the vector's buffer, and any debug-STL proxy.
    check(stack.counter.allocations == 2 + proxyAllocationsPerContainer);
    check(stack.counter.live() == 0);
};

auto makeOwnedThrowingConstructor =
    test("PMR.makeOwned.throwing_constructor_returns_the_memory") = []
{
    //Throwing allocates the exception object on the heap, so only the
    //resource's books are checked here.
    auto stack = StackResource<1024>();
    auto& resource = stack.counter;
    auto threw = false;

    try
    {
        auto owned = EA::PMR::makeOwned<ThrowsOnConstruction>(resource);
    }
    catch (const std::runtime_error&)
    {
        threw = true;
    }

    check(threw);
    check(resource.allocations == 1);
    check(resource.deallocations == 1);
    check(resource.bytesDeallocated == sizeof(ThrowsOnConstruction));
};

auto makeOwnedObjectPool =
    test("PMR.makeOwned.object_pool_fits_exactly_and_reuses_blocks") = []
{
    auto pool = EA::Allocators::ObjectPoolResource<Voice>(8);
    auto voices = std::vector<EA::PMR::Owned<Voice>>(8);
    auto firstAddresses = std::vector<void*>(8, nullptr);
    auto secondAddresses = std::vector<void*>(8, nullptr);

    //Each Voice is exactly one block, so not even the first allocation regrows.
    auto firstRoundCalls = countGlobalHeapCalls(
        [&]
        {
            for (auto index = 0; index < 8; ++index)
            {
                voices[(size_t) index] = EA::PMR::makeOwned<Voice>(pool);
                firstAddresses[(size_t) index] = voices[(size_t) index].get();
            }
        });

    void* released = voices[3].get();
    void* reused = nullptr;

    auto reuseCalls = countGlobalHeapCalls(
        [&]
        {
            voices[3].reset();
            voices[3] = EA::PMR::makeOwned<Voice>(pool, 0.5, 0.25f, 60);
            reused = voices[3].get();
        });

    auto secondRoundCalls = countGlobalHeapCalls(
        [&]
        {
            for (auto& voice: voices)
                voice.reset();

            for (auto index = 0; index < 8; ++index)
            {
                voices[(size_t) index] = EA::PMR::makeOwned<Voice>(pool);
                secondAddresses[(size_t) index] = voices[(size_t) index].get();
            }
        });

    check(firstRoundCalls == 0);
    check(reuseCalls == 0);
    check(secondRoundCalls == 0);
    check(reused == released);

    std::sort(firstAddresses.begin(), firstAddresses.end());
    std::sort(secondAddresses.begin(), secondAddresses.end());
    check(firstAddresses == secondAddresses);
};

auto makeOwnedBaseConversion =
    test("PMR.makeOwned.base_conversion_frees_with_derived_size_and_alignment") = []
{
    static_assert(
        std::is_convertible_v<EA::PMR::Owned<Derived>, EA::PMR::Owned<Base>>);
    static_assert(std::is_convertible_v<EA::PMR::Owned<PlainBase>,
                                        EA::PMR::Owned<const PlainBase>>);
    //No virtual destructor: converting would destroy only the base, so it
    //doesn't compile.
    static_assert(!std::is_convertible_v<EA::PMR::Owned<PlainDerived>,
                                         EA::PMR::Owned<PlainBase>>);

    auto stack = StackResource<1024>();
    auto destructions = 0;
    auto baseIsOffset = false;

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            EA::PMR::Owned<Base> owned =
                EA::PMR::makeOwned<Derived>(stack.counter, destructions, 7);
            baseIsOffset = static_cast<void*>(owned.get())
                           != stack.counter.lastAllocation.pointer;
        });

    check(heapCalls == 0);
    check(baseIsOffset);
    //~Derived and ~Base both ran.
    check(destructions == 2);
    check(stack.counter.live() == 0);
    check(stack.counter.lastDeallocation.pointer
          == stack.counter.lastAllocation.pointer);
    check(stack.counter.lastDeallocation.bytes == sizeof(Derived));
    check(stack.counter.lastDeallocation.alignment == alignof(Derived));
};

auto makeOwnedMoveOnly = test("PMR.makeOwned.is_move_only_and_moves_freely") = []
{
    static_assert(!std::is_copy_constructible_v<EA::PMR::Owned<Voice>>);
    static_assert(!std::is_copy_assignable_v<EA::PMR::Owned<Voice>>);
    static_assert(std::is_nothrow_move_constructible_v<EA::PMR::Owned<Voice>>);

    auto stack = StackResource<1024>();
    auto source = EA::PMR::makeOwned<Voice>(stack.counter);
    auto* original = source.get();
    auto movedFromIsNull = false;
    auto pointerKept = false;
    auto resourceKept = false;

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            auto moved = std::move(source);
            movedFromIsNull = source == nullptr;
            pointerKept = moved.get() == original;
            resourceKept = moved.get_deleter().resource == &stack.counter;

            //Hand it back so destruction happens outside the measured scope.
            source = std::move(moved);
        });

    check(heapCalls == 0);
    check(movedFromIsNull);
    check(pointerKept);
    check(resourceKept);
    check(stack.counter.allocations == 1);
    check(stack.counter.deallocations == 0);
    //unique_ptr isn't a container, so no debug-STL proxy is involved here.
};
