#pragma once

#include "Array.h"
#include "../Flags/CopyableAtomic.h"

namespace EA
{
/*
 * A lock free fifo meant to share big objects between threads
 *
 * The idea is push() can be called from any thread, but pull() provides a safe
 * pointer to use from a specific thread that remains untouched
 * until the newxt pull() call.
 *
 * This FIFO is meant for cases where we want the most up-to-date
 * version of the shared object when reading from it, but we're ok with 'skipping'
 * some of the in-between updates.
 *
 * This is good for something like a big vector that the processor needs to know about
 * constantly without knowing exactly what changed.
 *
 * For message passing between threads where you absolutely need every message read,
 * probably something else is useful
 */
template <typename T, int size = 5>
class Fifo
{
public:
    void push(const T& object)
    {
        auto loc = writePos.load();
        array[loc] = object;

        do
        {
            getNextLocation(loc);
        } while (loc == currentReadPosition.load());

        writePos.store(loc);
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
