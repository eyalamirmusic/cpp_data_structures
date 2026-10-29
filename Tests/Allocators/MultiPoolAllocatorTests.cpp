#include <Helpers/AllocationHelpers.h>
#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Allocators/MultiPoolAllocator.h>
#include <ea_data_structures/Allocators/PMR.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <list>
#include <memory>
#include <memory_resource>
#include <stdexcept>
#include <vector>

using namespace nano;
using EA::Allocators::DynamicMemory;
using EA::Allocators::MemoryPool;
using EA::Allocators::MemoryPoolResource;
using EA::Allocators::ObjectPoolResource;
using EA::Allocators::ResourceContainer;
using EA::TestHelpers::countGlobalHeapCalls;
using EA::TestHelpers::expectedHeapCalls;

namespace
{
bool isAligned(const void* ptr, std::size_t alignment)
{
    return reinterpret_cast<std::uintptr_t>(ptr) % alignment == 0;
}

bool allBytesEqual(const void* ptr, std::size_t size, unsigned char value)
{
    auto* bytes = static_cast<const unsigned char*>(ptr);

    for (auto index = std::size_t {0}; index < size; ++index)
    {
        if (bytes[index] != value)
            return false;
    }

    return true;
}

template <typename Func>
bool throwsRuntimeError(Func&& func)
{
    try
    {
        func();
    }
    catch (const std::runtime_error&)
    {
        return true;
    }

    return false;
}
} // namespace

//DynamicMemory

auto dynamicMemoryWithinCapacity =
    test("DynamicMemory.allocate_within_capacity_reuses_storage") = []
{
    auto memory = DynamicMemory(256);
    void* first = nullptr;
    void* second = nullptr;

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            first = memory.allocate(128);
            second = memory.allocate(256);
        });

    check(heapCalls == 0);
    check(first != nullptr);
    check(first == second);
    check(memory == first);
};

auto dynamicMemoryGrows =
    test("DynamicMemory.allocate_beyond_capacity_moves_to_a_bigger_buffer") = []
{
    auto memory = DynamicMemory(64);
    auto* original = memory.allocate(32);
    std::memset(original, 0xAB, 32);
    void* grown = nullptr;

    //Growing past the reserved capacity is a std::vector reallocation: one
    //allocation for the new buffer and one free for the old one.
    auto heapCalls = countGlobalHeapCalls([&] { grown = memory.allocate(128); });

    check(heapCalls == expectedHeapCalls(2));
    check(grown != original);
    check(memory == grown);
    check(!(memory == original));
    check(allBytesEqual(grown, 32, 0xAB));
};

auto dynamicMemoryOverAligned =
    test("DynamicMemory.over_aligned_allocate_returns_aligned_pointer") = []
{
    //48 bytes at 64-byte alignment need 48 + 63 bytes of block.
    static_assert(EA::Allocators::defaultAlignment
                  == __STDCPP_DEFAULT_NEW_ALIGNMENT__);
    check(DynamicMemory::getRequiredSpace(48, 64) == 111);
    check(DynamicMemory::getRequiredSpace(48, alignof(std::max_align_t)) == 48);
    check(DynamicMemory::getRequiredSpace(48, 16) == 48);

    auto memory = DynamicMemory(111);
    void* ptr = nullptr;

    auto heapCalls = countGlobalHeapCalls([&] { ptr = memory.allocate(48, 64); });

    check(heapCalls == 0);
    check(isAligned(ptr, 64));
    check(memory == ptr);
    std::memset(ptr, 0x11, 48);
    check(allBytesEqual(ptr, 48, 0x11));
};

//ResourceContainer

auto containerIsLifo = test("ResourceContainer.push_and_pop_back_are_lifo") = []
{
    auto container = ResourceContainer(4);
    check(container.empty());

    auto a = std::make_unique<DynamicMemory>(16);
    auto b = std::make_unique<DynamicMemory>(16);
    auto c = std::make_unique<DynamicMemory>(16);
    auto* rawA = a.get();
    auto* rawB = b.get();
    auto* rawC = c.get();

    container.push_back(std::move(a));
    container.push_back(std::move(b));
    container.push_back(std::move(c));
    check(container.resources.size() == 3);

    check(container.pop_back().get() == rawC);
    check(container.pop(0).get() == rawA);
    check(container.pop_back().get() == rawB);
    check(container.empty());
};

auto containerPopKeepsOrder =
    test("ResourceContainer.pop_from_middle_keeps_order") = []
{
    auto container = ResourceContainer(4);

    for (auto index = 0; index < 3; ++index)
        container.create(16);

    auto* first = container.resources[0].get();
    auto* middle = container.resources[1].get();
    auto* last = container.resources[2].get();

    check(container.pop(1).get() == middle);
    check(container.resources.size() == 2);
    check(container.resources[0].get() == first);
    check(container.resources[1].get() == last);
};

auto containerFindAndPop =
    test("ResourceContainer.find_and_pop_locates_block_by_address") = []
{
    auto container = ResourceContainer(4);

    for (auto index = 0; index < 3; ++index)
        container.create(32);

    auto* target = container.resources[1].get();
    auto* address = target->allocate(32);

    auto popped = container.findAndPop(address);
    check(popped.get() == target);
    check(container.resources.size() == 2);

    auto unknown = 0;
    check(throwsRuntimeError([&] { container.findAndPop(&unknown); }));
    check(container.resources.size() == 2);
};

auto containerReservedNoHeap =
    test("ResourceContainer.moving_blocks_within_reserve_skips_heap") = []
{
    auto container = ResourceContainer(8);

    for (auto index = 0; index < 4; ++index)
        container.create(64);

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            auto last = container.pop_back();
            auto first = container.pop(0);
            container.push_back(std::move(last));
            container.push_back(std::move(first));
        });

    check(heapCalls == 0);
    check(container.resources.size() == 4);
};

//MemoryPool

auto poolWarmNoHeap = test("MemoryPool.allocations_from_warm_pool_skip_heap") = []
{
    auto pool = MemoryPool(128, 8, 16);
    auto pointers = std::vector<void*>(8, nullptr);

    auto allocateCalls = countGlobalHeapCalls(
        [&]
        {
            for (auto& ptr: pointers)
                ptr = pool.allocate(128);
        });

    auto deallocateCalls = countGlobalHeapCalls(
        [&]
        {
            for (auto* ptr: pointers)
                pool.deallocate(ptr);
        });

    check(allocateCalls == 0);
    check(deallocateCalls == 0);
};

auto poolBlocksAreDistinct =
    test("MemoryPool.allocations_get_separate_non_overlapping_blocks") = []
{
    constexpr auto blockSize = std::size_t {64};
    constexpr auto count = 8;

    auto pool = MemoryPool((int) blockSize, count, 16);
    auto pointers = std::vector<unsigned char*>();

    for (auto index = 0; index < count; ++index)
    {
        auto* ptr = static_cast<unsigned char*>(pool.allocate(blockSize));
        std::memset(ptr, index + 1, blockSize);
        pointers.push_back(ptr);
    }

    auto disjoint = true;

    for (auto i = 0; i < count; ++i)
    {
        for (auto j = i + 1; j < count; ++j)
        {
            auto* a = pointers[(size_t) i];
            auto* b = pointers[(size_t) j];

            if (a < b + blockSize && b < a + blockSize)
                disjoint = false;
        }
    }

    check(disjoint);

    for (auto index = 0; index < count; ++index)
    {
        check(allBytesEqual(
            pointers[(size_t) index], blockSize, (unsigned char) (index + 1)));
    }
};

auto poolGrowsWhenExhausted =
    test("MemoryPool.exhausted_pool_creates_a_block_then_keeps_it") = []
{
    auto pool = MemoryPool(64, 2, 16);
    auto pointers = std::vector<void*>(4, nullptr);

    auto warmCalls = countGlobalHeapCalls(
        [&]
        {
            pointers[0] = pool.allocate(64);
            pointers[1] = pool.allocate(64);
        });

    //Once the free list is empty, each allocation creates one block: one
    //allocation for the DynamicMemory and one for its reserved storage.
    auto thirdCalls = countGlobalHeapCalls([&] { pointers[2] = pool.allocate(64); });
    auto fourthCalls =
        countGlobalHeapCalls([&] { pointers[3] = pool.allocate(64); });

    check(warmCalls == 0);
    check(thirdCalls == expectedHeapCalls(2));
    check(fourthCalls == expectedHeapCalls(2));

    //The grown blocks stay in the pool, so the next round is heap-free.
    auto secondRoundCalls = countGlobalHeapCalls(
        [&]
        {
            for (auto* ptr: pointers)
                pool.deallocate(ptr);

            for (auto& ptr: pointers)
                ptr = pool.allocate(64);
        });

    check(secondRoundCalls == 0);
};

auto poolReusesLastFreed =
    test("MemoryPool.deallocated_block_is_handed_out_next") = []
{
    auto pool = MemoryPool(64, 4, 16);
    auto* first = pool.allocate(64);
    auto* other = pool.allocate(64);
    void* reused = nullptr;

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            pool.deallocate(first);
            reused = pool.allocate(64);
        });

    check(heapCalls == 0);
    check(reused == first);
    check(reused != other);
};

auto poolOversizeOnEmpty =
    test("MemoryPool.oversized_request_on_empty_pool_gets_a_fitting_block") = []
{
    auto pool = MemoryPool(64, 0, 16);
    void* ptr = nullptr;

    //The new block is sized max(request, objectSize), so the request fits and
    //the block isn't resized afterwards: still exactly one block creation.
    auto heapCalls = countGlobalHeapCalls([&] { ptr = pool.allocate(1000); });

    check(heapCalls == expectedHeapCalls(2));
    std::memset(ptr, 0x5A, 1000);
    check(allBytesEqual(ptr, 1000, 0x5A));
};

auto poolOversizeGrowsFreeBlock =
    test("MemoryPool.oversized_request_grows_a_free_block_and_it_stays_grown") = []
{
    auto pool = MemoryPool(64, 1, 16);
    void* ptr = nullptr;

    //The only free block holds 64 bytes, so it has to reallocate to fit 1000.
    auto growCalls = countGlobalHeapCalls([&] { ptr = pool.allocate(1000); });

    check(growCalls == expectedHeapCalls(2));
    std::memset(ptr, 0x3C, 1000);
    check(allBytesEqual(ptr, 1000, 0x3C));

    auto reuseCalls = countGlobalHeapCalls(
        [&]
        {
            pool.deallocate(ptr);
            ptr = pool.allocate(1000);
        });

    check(reuseCalls == 0);
};

auto poolBookkeepingGrows =
    test("MemoryPool.exceeding_reserved_objects_grows_bookkeeping") = []
{
    //4 free blocks, but room for only 2 entries in the used list.
    auto pool = MemoryPool(32, 4, 2);

    auto withinReserveCalls = countGlobalHeapCalls(
        [&]
        {
            pool.allocate(32);
            pool.allocate(32);
        });

    //No new block is needed, but the used list's vector regrows.
    auto beyondReserveCalls = countGlobalHeapCalls([&] { pool.allocate(32); });

    check(withinReserveCalls == 0);
    check(beyondReserveCalls == expectedHeapCalls(2));
};

auto poolRejectsUnknownPointer =
    test("MemoryPool.deallocating_foreign_pointer_throws") = []
{
    auto pool = MemoryPool(64, 2, 16);
    auto foreign = 0;

    check(throwsRuntimeError([&] { pool.deallocate(&foreign); }));
};

auto poolRejectsDoubleFree = test("MemoryPool.double_deallocate_throws") = []
{
    auto pool = MemoryPool(64, 2, 16);
    auto* ptr = pool.allocate(64);
    pool.deallocate(ptr);

    check(throwsRuntimeError([&] { pool.deallocate(ptr); }));
};

auto poolDefaults = test("MemoryPool.defaults_prefill_sixteen_blocks") = []
{
    auto pool = MemoryPool(64);

    auto prefilledCalls = countGlobalHeapCalls(
        [&]
        {
            for (auto index = 0; index < 16; ++index)
                pool.allocate(64);
        });

    auto seventeenthCalls = countGlobalHeapCalls([&] { pool.allocate(64); });

    check(prefilledCalls == 0);
    check(seventeenthCalls == expectedHeapCalls(2));
};

auto poolOverAlignedRoundTrip =
    test("MemoryPool.over_aligned_allocation_deallocates_and_is_reused") = []
{
    auto pool = MemoryPool(128, 4, 16);
    void* first = nullptr;
    void* second = nullptr;

    //128-byte blocks fit 48 bytes at 64-byte alignment (48 + 63 = 111).
    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            first = pool.allocate(48, 64);
            pool.deallocate(first);
            second = pool.allocate(48, 64);
        });

    check(heapCalls == 0);
    check(isAligned(first, 64));
    check(first == second);
};

auto poolOverAlignedOnEmpty =
    test("MemoryPool.over_aligned_request_on_empty_pool_gets_room_to_align") = []
{
    auto pool = MemoryPool(64, 0, 16);
    void* ptr = nullptr;

    //The new block is sized for the alignment slack too, so it isn't regrown.
    auto heapCalls = countGlobalHeapCalls([&] { ptr = pool.allocate(64, 64); });

    check(heapCalls == expectedHeapCalls(2));
    check(isAligned(ptr, 64));
};

//Requests at or below defaultAlignment take no slack, so the pool relies on
//every block start honouring __STDCPP_DEFAULT_NEW_ALIGNMENT__.
auto poolDefaultAlignment =
    test("MemoryPool.blocks_are_aligned_to_default_new_alignment") = []
{
    auto pool = MemoryPool(24, 32, 64);
    auto aligned = true;

    for (auto size = std::size_t {1}; size <= 32; ++size)
    {
        if (!isAligned(pool.allocate(size), EA::Allocators::defaultAlignment))
            aligned = false;
    }

    check(aligned);
};

auto poolSixteenAlignedNoSlack =
    test("MemoryPool.sixteen_aligned_request_takes_no_slack") = []
{
    //48-byte blocks hold a 48-byte, 16-aligned request as-is; with any
    //alignment slack the block would have to regrow on the heap.
    auto pool = MemoryPool(48, 1, 4);
    void* ptr = nullptr;

    auto heapCalls = countGlobalHeapCalls([&] { ptr = pool.allocate(48, 16); });

    check(heapCalls == 0);
    check(isAligned(ptr, 16));
};

//MemoryPoolResource

auto resourceEquality = test("MemoryPoolResource.equal_only_to_itself") = []
{
    auto first = MemoryPoolResource(64, 1, 4);
    auto second = MemoryPoolResource(64, 1, 4);

    check(first.is_equal(first));
    check(!first.is_equal(second));
    check(first != second);
};

auto resourceRoundTrip =
    test("MemoryPoolResource.allocate_deallocate_round_trip_reuses_block") = []
{
    auto resource = MemoryPoolResource(256, 4, 16);
    void* first = nullptr;
    void* second = nullptr;

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            first = resource.allocate(200);
            resource.deallocate(first, 200);
            second = resource.allocate(100);
        });

    check(heapCalls == 0);
    check(first == second);
};

auto resourcePmrVectorNoHeap =
    test("MemoryPoolResource.pmr_vector_growth_skips_heap") = []
{
    auto resource = MemoryPoolResource(4096, 16, 64);
    auto size = std::size_t {0};
    auto sum = 0;
    auto usedResource = static_cast<std::pmr::memory_resource*>(nullptr);

    //1000 ints fit in a 4096-byte block, so every regrowth takes a free block
    //and returns the previous one; destruction returns the last one too.
    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            auto values = std::pmr::vector<int>(&resource);

            for (auto index = 0; index < 1000; ++index)
                values.push_back(index);

            size = values.size();
            usedResource = values.get_allocator().resource();

            for (auto value: values)
                sum += value;
        });

    check(heapCalls == 0);
    check(size == 1000);
    check(sum == 999 * 1000 / 2);
    check(usedResource == &resource);
};

auto resourceVectorsSpanBlocks =
    test("MemoryPoolResource.concurrent_vectors_use_separate_blocks") = []
{
    auto resource = MemoryPoolResource(1024, 8, 16);
    auto vectors = std::vector<std::pmr::vector<int>>();
    vectors.reserve(4);

    for (auto index = 0; index < 4; ++index)
        vectors.emplace_back(&resource);

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            for (auto index = 0; index < 4; ++index)
                vectors[(size_t) index].assign(256, index);
        });

    check(heapCalls == 0);

    for (auto i = 0; i < 4; ++i)
    {
        for (auto j = i + 1; j < 4; ++j)
            check(vectors[(size_t) i].data() != vectors[(size_t) j].data());
    }

    for (auto index = 0; index < 4; ++index)
    {
        auto intact = true;

        for (auto value: vectors[(size_t) index])
        {
            if (value != index)
                intact = false;
        }

        check(intact);
    }
};

auto resourceVectorOutgrowsBlock =
    test("MemoryPoolResource.vector_outgrowing_block_size_touches_heap") = []
{
    auto resource = MemoryPoolResource(256, 4, 16);
    auto values = std::pmr::vector<int>(&resource);

    auto fitsCalls = countGlobalHeapCalls([&] { values.reserve(64); });

    //512 bytes no longer fit a 256-byte block: the block taken from the free
    //list is regrown (one allocation, one free). The old 256-byte buffer goes
    //back to the pool, which doesn't touch the heap.
    auto outgrowCalls = countGlobalHeapCalls([&] { values.reserve(128); });

    check(fitsCalls == 0);
    check(outgrowCalls == expectedHeapCalls(2));
    check(values.capacity() >= 128);
};

//48-byte blocks are too small for 48 bytes at 64-byte alignment, so each one
//regrows; what matters here is that every pointer comes back 64-aligned.
auto resourceOverAligned =
    test("MemoryPoolResource.honours_over_aligned_requests") = []
{
    auto resource = MemoryPoolResource(48, 32, 64);
    auto pointers = std::vector<void*>();
    auto aligned = true;

    for (auto index = 0; index < 32; ++index)
    {
        auto* ptr = resource.allocate(48, 64);
        pointers.push_back(ptr);

        if (!isAligned(ptr, 64))
            aligned = false;
    }

    check(aligned);

    for (auto* ptr: pointers)
        resource.deallocate(ptr, 48, 64);
};

auto resourceDefaults =
    test("MemoryPoolResource.defaults_prefill_sixteen_64k_blocks") = []
{
    auto resource = MemoryPoolResource();
    auto blockSize = std::size_t {64 * 1024};

    auto prefilledCalls = countGlobalHeapCalls(
        [&]
        {
            for (auto index = 0; index < 16; ++index)
                (void) resource.allocate(blockSize);
        });

    auto seventeenthCalls =
        countGlobalHeapCalls([&] { (void) resource.allocate(blockSize); });

    check(prefilledCalls == 0);
    check(seventeenthCalls == expectedHeapCalls(2));
};

//ObjectPoolResource

namespace
{
struct Particle
{
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
    int id = 0;
};

struct alignas(64) CacheLine
{
    std::byte data[64] {};
};

//Every pointer aligned to `alignment`, and no two closer than `size` bytes.
bool alignedAndDisjoint(std::vector<void*> pointers,
                        std::size_t size,
                        std::size_t alignment)
{
    std::sort(pointers.begin(), pointers.end());

    for (auto index = std::size_t {0}; index < pointers.size(); ++index)
    {
        if (!isAligned(pointers[index], alignment))
            return false;

        if (index > 0)
        {
            auto gap = static_cast<std::byte*>(pointers[index])
                       - static_cast<std::byte*>(pointers[index - 1]);

            if (gap < (std::ptrdiff_t) size)
                return false;
        }
    }

    return true;
}
} // namespace

auto objectPoolDefaultCount =
    test("ObjectPoolResource.default_count_serves_1024_objects_without_heap") = []
{
    using Pool = ObjectPoolResource<Particle>;
    static_assert(Pool::blockSize == (int) sizeof(Particle));

    auto pool = Pool();
    auto pointers = std::vector<void*>(1024, nullptr);

    auto allocateCalls = countGlobalHeapCalls(
        [&]
        {
            for (auto& ptr: pointers)
                ptr = pool.allocate(sizeof(Particle), alignof(Particle));
        });

    check(allocateCalls == 0);
    check(alignedAndDisjoint(pointers, sizeof(Particle), alignof(Particle)));

    auto deallocateCalls = countGlobalHeapCalls(
        [&]
        {
            for (auto* ptr: pointers)
                pool.deallocate(ptr, sizeof(Particle), alignof(Particle));
        });

    check(deallocateCalls == 0);
};

auto objectPoolCustomCount =
    test("ObjectPoolResource.custom_count_then_next_allocation_grows") = []
{
    auto pool = ObjectPoolResource<Particle>(8);

    auto withinCountCalls = countGlobalHeapCalls(
        [&]
        {
            for (auto index = 0; index < 8; ++index)
                (void) pool.allocate(sizeof(Particle), alignof(Particle));
        });

    //The 9th needs a fresh block (the DynamicMemory and its storage) and the
    //used list, reserved for exactly 8, regrows (new buffer, old one freed).
    auto ninthCalls = countGlobalHeapCalls(
        [&] { (void) pool.allocate(sizeof(Particle), alignof(Particle)); });

    check(withinCountCalls == 0);
    check(ninthCalls == expectedHeapCalls(4));
};

auto objectPoolOverAligned =
    test("ObjectPoolResource.over_aligned_type_comes_back_aligned") = []
{
    using Pool = ObjectPoolResource<CacheLine>;
    static_assert(Pool::blockSize == 64 + 64 - 1);

    //16-byte alignment is what operator new already gives, so no slack.
    struct alignas(16) Vec4
    {
        float values[4];
    };
    static_assert(ObjectPoolResource<Vec4>::blockSize == 16);

    auto pool = Pool(32);
    auto pointers = std::vector<void*>(32, nullptr);

    auto heapCalls = countGlobalHeapCalls(
        [&]
        {
            for (auto& ptr: pointers)
                ptr = pool.allocate(sizeof(CacheLine), alignof(CacheLine));
        });

    check(heapCalls == 0);
    check(alignedAndDisjoint(pointers, sizeof(CacheLine), 64));
};

//allocate_shared puts the control block and the Particle in one allocation,
//which is bigger than sizeof(Particle). Each block regrows on the heap the
//first time it serves one; after that the grown block is reused heap-free.
auto objectPoolMakeShared =
    test("ObjectPoolResource.make_shared_regrows_blocks_once") = []
{
    auto pool = ObjectPoolResource<Particle>(8);
    auto objects = std::vector<std::shared_ptr<Particle>>(8);

    auto firstRoundCalls = countGlobalHeapCalls(
        [&]
        {
            for (auto& object: objects)
                object = EA::PMR::makeShared<Particle>(pool);

            for (auto& object: objects)
                object.reset();
        });

    auto secondRoundCalls = countGlobalHeapCalls(
        [&]
        {
            for (auto& object: objects)
                object = EA::PMR::makeShared<Particle>(pool);

            for (auto& object: objects)
                object.reset();
        });

    check(firstRoundCalls == expectedHeapCalls(16));
    check(secondRoundCalls == 0);
};

//Same story for list nodes, which carry two links next to the Particle.
auto objectPoolList = test("ObjectPoolResource.pmr_list_regrows_blocks_once") = []
{
    auto pool = ObjectPoolResource<Particle>(8);
    auto particles = std::pmr::list<Particle>(&pool);

    auto fill = [&]
    {
        for (auto index = 0; index < 8; ++index)
            particles.push_back(Particle {0.f, 0.f, 0.f, index});
    };

    auto firstRoundCalls = countGlobalHeapCalls(
        [&]
        {
            fill();
            particles.clear();
        });

    auto secondRoundCalls = countGlobalHeapCalls(
        [&]
        {
            fill();
            particles.clear();
        });

    check(firstRoundCalls == expectedHeapCalls(16));
    check(secondRoundCalls == 0);
};
