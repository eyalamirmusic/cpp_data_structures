#pragma once

namespace EA::TestHelpers
{
//Aggregate counts of the lifecycle operations that happen to any
//OperationTracker sharing the static counters object. Use live() to check
//for leaks and the individual fields to assert exactly which flavor of
//construction/assignment occurred.
struct OperationCounters
{
    int defaultConstructions = 0;
    int valueConstructions = 0;
    int copyConstructions = 0;
    int moveConstructions = 0;
    int copyAssignments = 0;
    int moveAssignments = 0;
    int destructions = 0;

    int totalConstructions() const noexcept
    {
        return defaultConstructions + valueConstructions + copyConstructions
             + moveConstructions;
    }

    //Net constructions - destructions since the last reset(). This equals the
    //number of live OperationTracker instances only when all of them were
    //constructed after the most recent reset(); after a mid-test reset()
    //this is a delta, not an absolute count.
    int live() const noexcept { return totalConstructions() - destructions; }
};

//Instrumented value type for testing container lifecycle behavior. Every
//special-member operation bumps a counter in the shared OperationCounters,
//so tests can assert exact numbers of copies, moves, and destructions.
//
//Tests MUST call OperationTracker::reset() at the start since the counters
//are a single static object shared across the whole test binary.
class OperationTracker
{
public:
    inline static OperationCounters counters {};

    static void reset() noexcept { counters = {}; }

    OperationTracker() noexcept { ++counters.defaultConstructions; }

    explicit OperationTracker(int v) noexcept
        : value(v)
    {
        ++counters.valueConstructions;
    }

    OperationTracker(const OperationTracker& other) noexcept
        : value(other.value)
    {
        ++counters.copyConstructions;
    }

    OperationTracker(OperationTracker&& other) noexcept
        : value(other.value)
    {
        other.movedFrom = true;
        ++counters.moveConstructions;
    }

    OperationTracker& operator=(const OperationTracker& other) noexcept
    {
        value = other.value;
        ++counters.copyAssignments;
        return *this;
    }

    OperationTracker& operator=(OperationTracker&& other) noexcept
    {
        value = other.value;
        other.movedFrom = true;
        ++counters.moveAssignments;
        return *this;
    }

    ~OperationTracker() noexcept { ++counters.destructions; }

    int getValue() const noexcept { return value; }
    bool wasMovedFrom() const noexcept { return movedFrom; }

    friend bool
        operator==(const OperationTracker& lhs, const OperationTracker& rhs) noexcept
    {
        return lhs.value == rhs.value;
    }

    friend bool
        operator<(const OperationTracker& lhs, const OperationTracker& rhs) noexcept
    {
        return lhs.value < rhs.value;
    }

private:
    int value = 0;
    bool movedFrom = false;
};
} // namespace EA::TestHelpers
