#pragma once

#include "Array.h"
#include "../Flags/CopyableAtomic.h"

#include <cassert>

namespace EA
{
/*
 * A lock free fifo meant to share big objects between threads
 *
 * The idea is push() can be called from any thread, but pull() provides a safe
 * pointer to use from a specific thread that remains untouched
 * until the next pull() call.
 *
 * This FIFO is meant for cases where we want the most up-to-date
 * version of the shared object when reading from it, but we're ok with 'skipping'
 * some of the in-between updates.
 *
 * If the writer gets a full buffer ahead of the reader the fifo is full:
 * tryPush() returns false and drops the value, push() asserts in debug.
 * pull() always returns the most recent successfully pushed value.
 *
 * This is good for something like a big vector that the processor needs to know about
 * constantly without knowing exactly what changed.
 *
 * For message passing between threads where you absolutely need every message read,
 * probably something else is useful
 */
template <typename T, int size = 50>
class Fifo
{
public:
    //One slot is held by the reader, one holds the last completed write,
    //and at least one more is needed to have room to write into
    static_assert(size >= 3, "Fifo needs at least 3 slots");

    //Attempts to push. Returns false and drops the value if the fifo is full
    //because the reader hasn't pulled recently enough
    bool tryPush(const T& object)
    {
        auto loc = writePos.load();
        auto next = loc;
        getNextLocation(next);

        if (next == currentReadPosition.load())
            return false;

        array[loc] = object;
        writePos.store(next);
        return true;
    }

    void push(const T& object)
    {
        [[maybe_unused]] auto pushed = tryPush(object);

        //The fifo was full, so this value was dropped and the reader will not
        //see it. Push less often, pull more often, or use a bigger size
        assert(pushed);
    }

    //Fills the FIFO with values
    void fill(const T& data) noexcept { array.fill(data); }

    T& pull() noexcept
    {
        auto readPos = getReadLocation();
        currentReadPosition.store(readPos);
        return array[readPos];
    }

private:
    int getReadLocation() noexcept
    {
        auto readPos = writePos.load() - 1;

        if (readPos == -1)
            readPos = array.getLastElementIndex();

        return readPos;
    }

    static void getNextLocation(int& prevLocation)
    {
        prevLocation++;

        if (prevLocation == size)
            prevLocation = 0;
    }

    Atomic<int> writePos {0};
    Atomic<int> currentReadPosition {-1};

    Array<T, size> array;
};

} // namespace EA
