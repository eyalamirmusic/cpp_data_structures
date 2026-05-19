#pragma once

#include "CallbackFunc.h"
#include "../Structures/Vector.h"
#include <type_traits>

//A minimal Listener/Broadcaster pair built on top of CallbackFunc.
//
//Usage:
//    Broadcaster broadcaster;
//    Listener    listener { broadcaster, [] { /* react */ } };
//    broadcaster.trigger();   // invokes listener's callback
//
//Lifetime rules:
//  - A Listener is bound to one Broadcaster at construction and stays bound
//    until destroyed; there is no add/remove API on the broadcaster side.
//  - Listeners are expected to outlive the broadcaster they observe. When a
//    Broadcaster dies first it clears the back-pointer on each of its
//    listeners so that the listener's destructor becomes a no-op.
//  - Copying or moving a Broadcaster does NOT carry listeners across; the
//    new/assigned-to instance keeps whatever listener set it already had
//    (empty for a fresh copy/move-constructed instance).
//
//Re-entrancy during trigger():
//  - It is safe for a listener's callback to destroy itself, destroy other
//    listeners, or construct new listeners attached to the same broadcaster.
//  - Listeners attached during a trigger() do NOT fire on the in-flight
//    pass; they fire on the next trigger().
//  - Listeners removed during a trigger() are not invoked again on the
//    in-flight pass even if their index has not been reached yet.
//  - Destroying the broadcaster from inside a callback is NOT supported.

namespace EA
{

class Broadcaster;

class Listener
{
public:
    enum class Modes
    {
        TriggerNow, // invoke the callback once at construction
        TriggerOnEvent // only invoke on broadcaster.trigger()
    };

    template <typename T, typename Callable>
    Listener(T& source, Callable&& callbackToUse, Modes mode = Modes::TriggerNow);

    ~Listener();

    Listener(const Listener&) = delete;
    Listener(Listener&&) = delete;
    Listener& operator=(const Listener&) = delete;
    Listener& operator=(Listener&&) = delete;

private:
    friend class Broadcaster;

    void invoke() const { callback(); }

    CallbackFunc callback;
    Broadcaster* broadcaster = nullptr;
};

class Broadcaster
{
public:
    Broadcaster() = default;

    Broadcaster(const Broadcaster&) noexcept {}
    Broadcaster(Broadcaster&&) noexcept {}
    Broadcaster& operator=(const Broadcaster&) noexcept { return *this; }
    Broadcaster& operator=(Broadcaster&&) noexcept { return *this; }

    void trigger()
    {
        flushPendingAdds();

        for (auto* listener: listeners)
        {
            if (listener != nullptr)
                listener->invoke();
        }

        listeners.eraseIf([](const Listener* l) { return l == nullptr; });
    }

private:
    friend class Listener;

    void attach(Listener* listener) { listenersToAdd.add(listener); }

    void detach(const Listener* listener)
    {
        for (auto& slot: listeners)
        {
            if (slot == listener)
            {
                slot = nullptr;
                return;
            }
        }

        for (auto& slot: listenersToAdd)
        {
            if (slot == listener)
            {
                slot = nullptr;
                return;
            }
        }
    }

    void flushPendingAdds()
    {
        for (auto* listener: listenersToAdd)
        {
            if (listener != nullptr)
                listeners.add(listener);
        }

        listenersToAdd.clear();
    }

    Vector<Listener*> listeners;
    Vector<Listener*> listenersToAdd;
};

template <typename T, typename Callable>
Listener::Listener(T& source, Callable&& callbackToUse, Modes mode)
    : callback(std::forward<Callable>(callbackToUse))
    , broadcaster(nullptr)
{
    if constexpr (std::is_base_of_v<Broadcaster, T>)
        broadcaster = &source;
    else
        broadcaster = &source.getBroadcaster();

    if (mode == Modes::TriggerNow)
        callback();

    broadcaster->attach(this);
}

inline Listener::~Listener()
{
    if (broadcaster != nullptr)
        broadcaster->detach(this);
}

struct BroadcasterOwner
{
    virtual ~BroadcasterOwner() = default;
    virtual Broadcaster& getBroadcaster() { return broadcaster; }

    void trigger() { broadcaster.trigger(); }

    Broadcaster broadcaster;
};

} // namespace EA
