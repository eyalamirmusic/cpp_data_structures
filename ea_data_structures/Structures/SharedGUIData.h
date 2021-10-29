#pragma once

#include "Fifo.h"

namespace EA
{
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
    void blockStarted() { rt = &fifo.pull(); }

    //Call this from the GUI thread to update the data
    void push() { fifo.push(data); }

    //Useful operators to call from the GUI only to reach the shared object
    T* operator->() { return &data; }
    T& operator*() { return data; }

    //Another way to access the realtime object. Only valid during processBlock.
    T& getRealTime()
    {
        blockStarted();
        return *rt;
    }

    //Safe to use pointer, but only during the process block. See blockStarted()
    T* rt = nullptr;

private:
    T data;
    Fifo<T, fifoSize> fifo;
};

//Similar to SharedWithRealTime, but a non-owning version of the GUI data
template <typename T, int fifoSize = 5>
class GUIToRealTime
{
public:
    explicit GUIToRealTime(T& dataToUse)
        : data(dataToUse)
    {
        push();
    }

    //Call at the start of the block. This will make the RT pointer safe to use
    void blockStarted() { rt = &fifo.pull(); }

    //Call this from the GUI thread to update the data
    void push() { fifo.push(data); }

    //Useful operators to call from the GUI only to reach the shared object
    T* operator->() { return &data; }
    T& operator*() { return data; }

    //Another way to access the realtime object. Only valid during processBlock.
    T& getRealTime()
    {
        blockStarted();
        return *rt;
    }

    //Safe to use pointer, but only during the process block. See blockStarted()
    T* rt = nullptr;

private:
    T& data;
    Fifo<T, fifoSize> fifo;
};

} // namespace EA
