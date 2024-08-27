#pragma once

#include "Fifo.h"

namespace EA
{
//Similar to SharedWithRealTime, but a non-owning version of the GUI data
template <typename T, int fifoSize = 5>
class GUIToRealTime
{
public:
    explicit GUIToRealTime(const T& dataToUse = {}) { push(dataToUse); }

    //Call at the start of the block. This will make the RT pointer safe to use
    void blockStarted() noexcept { rt = &fifo.pull(); }

    //Call this from the GUI thread to update the data
    void push(const T& data) { fifo.push(data); }

    //Fills the FIFO with values of certain type:
    void fill(const T& data) noexcept { fifo.fill(data); }

    auto getPushFunc(const T& data)
    {
        return [this, d = &data] { push(*d); };
    }

    //Another way to access the realtime object. Only valid during processBlock.
    T& getRealTime() noexcept
    {
        blockStarted();
        return *rt;
    }

private:
    Fifo<T, fifoSize> fifo;

    //Safe to use pointer, but only during the process block. See blockStarted()
    T* rt = nullptr;
};

/*This is a useful object for classic cases of sharing random data
 * that gets edited in the GUI but read from the processor
 *
 * To use, create one SharedWithRealTime<T> in your processor. You can freely pass
 * a reference to it, but remember that you must call push() for the updates
 * to register in the processor
 *
 * On the processor side, you need to call blockStarted() at the start of the block
 * And from then on the rt object will be safe to use with no thread contentions
*/
template <typename T, int fifoSize = 5>
class SharedWithRealTime
{
public:
    SharedWithRealTime() { push(); }

    //Call at the start of the block. This will make the RT pointer safe to use
    void blockStarted() noexcept { fifo.blockStarted(); }

    //Call this from the GUI thread to update the data
    void push() { fifo.push(data); }

    //Useful operators to call from the GUI only to reach the shared object
    T* operator->() { return &data; }
    T& operator*() { return data; }

    //Another way to access the realtime object. Only valid during processBlock.
    T& getRealTime() noexcept { return fifo.getRealTime(); }

private:
    T data;
    GUIToRealTime<T, fifoSize> fifo;
};

template <typename T, int FifoSize = 5>
struct RealTimeToGUI
{
    RealTimeToGUI() = default;
    RealTimeToGUI(const T& initalValue) { push(initalValue); }

    void push(const T& data) noexcept
    {
        fifo.push(data);
        updateFlag.update();
    }

    T& pull() { return fifo.pull(); }

    AtomicFlag updateFlag;
    Fifo<T, FifoSize> fifo;
};

} // namespace EA
