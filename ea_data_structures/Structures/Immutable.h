#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include "Vector.h"

//Persistent (structurally shared) trees of immutable nodes.
//
//An Immutable<T> is a one-pointer, refcounted handle to a const T. Copies share
//the node. The only way to change a tree is through an edit:
//
//    project.update([&](const Project& p) { muteByName(p.rootTrack, "Drums"); });
//
//Inside the edit, user code reads plain `const T&` values and descends with
//forEach(vector, fn) / visit(handle, fn). Each descent pushes a frame on a
//thread-local stack recording where the child sits in its parent. write(x) then
//walks those frames back up and lazily copies x's node and all its ancestors
//(path copying), each at most once per edit. Untouched subtrees keep identity.
//When update() returns the root is rebound to the new tree; if nothing was
//written it stays the same node. If the edit throws, the root is left untouched.
//
//Stale-read rule: write(node) returns a reference to the *draft*. Reads through
//the `node` reference you passed in still see the original value, so read back
//through the returned reference if you need your own writes.
//
//Preconditions (checked with EA_IMMUTABLE_ASSERT):
//- write(x), forEach and visit only work on objects living inline (directly or
//  nested in plain members) inside the value of a node that is currently being
//  visited, i.e. reached via update()/forEach()/visit(). Objects in other heap
//  containers (std::vector, std::unique_ptr...) inside a node are not supported.
//- Don't structurally modify (add/insert/remove/clear) a vector while forEach
//  iterates it. Targeted element edits through operator[] are fine.
//- Don't update() a root from inside its own update().

//Define EA_IMMUTABLE_ASSERT yourself, or define EA_IMMUTABLE_ASSERT_THROWS to
//turn violations into EA::ImmutablePreconditionError exceptions.
#ifndef EA_IMMUTABLE_ASSERT
#if defined(EA_IMMUTABLE_ASSERT_THROWS)
#define EA_IMMUTABLE_ASSERT(condition)                                              \
    ((condition) ? (void) 0 : throw ::EA::ImmutablePreconditionError(#condition))
#else
#define EA_IMMUTABLE_ASSERT(condition) assert(condition)
#endif
#endif

namespace EA
{
struct ImmutablePreconditionError : std::logic_error
{
    using std::logic_error::logic_error;
};

template <typename T>
class Immutable;

template <typename T>
class ImmutableVector;

namespace ImmutableDetail
{
struct Access;

//Type-erased node: intrusive refcount + the id of the edit allowed to mutate it
struct NodeBase
{
    NodeBase() = default;
    NodeBase(const NodeBase&) = delete;
    NodeBase& operator=(const NodeBase&) = delete;
    virtual ~NodeBase() = default;

    virtual NodeBase* clone() const = 0;

    std::atomic<int> refCount {1};
    std::uint64_t owner = 0;
};

template <typename T>
struct Node final : NodeBase
{
    template <typename... Args>
    explicit Node(std::in_place_t, Args&&... args)
        : value(construct(std::forward<Args>(args)...))
    {
    }

    NodeBase* clone() const override { return new Node(std::in_place, value); }

    template <typename... Args>
    static T construct(Args&&... args)
    {
        if constexpr (std::is_constructible_v<T, Args...>)
            return T(std::forward<Args>(args)...);
        else
            return T {std::forward<Args>(args)...};
    }

    T value;
};

inline void retain(NodeBase* node) noexcept
{
    if (node != nullptr)
        node->refCount.fetch_add(1, std::memory_order_relaxed);
}

inline void release(NodeBase* node) noexcept
{
    if (node != nullptr
        && node->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        delete node;
}

inline bool isUnique(const NodeBase* node) noexcept
{
    return node->refCount.load(std::memory_order_acquire) == 1;
}
} // namespace ImmutableDetail

//The non-template part of every handle: one pointer, refcounting written once
class ImmutableBase
{
public:
    ImmutableBase() = default;

    ImmutableBase(const ImmutableBase& other) noexcept
        : node(other.node)
    {
        ImmutableDetail::retain(node);
    }

    ImmutableBase(ImmutableBase&& other) noexcept
        : node(std::exchange(other.node, nullptr))
    {
    }

    ImmutableBase& operator=(const ImmutableBase& other) noexcept
    {
        ImmutableDetail::retain(other.node);
        adopt(other.node);
        return *this;
    }

    ImmutableBase& operator=(ImmutableBase&& other) noexcept
    {
        if (this != &other)
            adopt(std::exchange(other.node, nullptr));

        return *this;
    }

    ~ImmutableBase() { ImmutableDetail::release(node); }

    //True if both handles point at the very same node (or are both default)
    bool isSameAs(const ImmutableBase& other) const noexcept
    {
        return node == other.node;
    }

protected:
    explicit ImmutableBase(ImmutableDetail::NodeBase* adopted) noexcept
        : node(adopted)
    {
    }

    //Takes over a reference the caller already holds
    void adopt(ImmutableDetail::NodeBase* newNode) noexcept
    {
        ImmutableDetail::release(std::exchange(node, newNode));
    }

    ImmutableDetail::NodeBase* node = nullptr;

    friend struct ImmutableDetail::Access;
};

namespace ImmutableDetail
{
struct Access
{
    static NodeBase* get(const ImmutableBase& handle) noexcept
    {
        return handle.node;
    }

    static void adopt(ImmutableBase& handle, NodeBase* node) noexcept
    {
        handle.adopt(node);
    }

    template <typename T>
    static Vector<Immutable<T>>& handles(ImmutableVector<T>& vector) noexcept
    {
        return vector.handles;
    }
};

//What a frame needs to know about the value type it points at
struct TypeOps
{
    NodeBase* (*createDefault)();
    void* (*valueOf)(NodeBase*);
    std::size_t size;
};

template <typename T>
NodeBase* createDefaultNode()
{
    return new Node<T>(std::in_place);
}

template <typename T>
void* nodeValue(NodeBase* node)
{
    return &static_cast<Node<T>*>(node)->value;
}

template <typename T>
inline constexpr TypeOps typeOps {&createDefaultNode<T>, &nodeValue<T>, sizeof(T)};

//Returns the handle stored in a parent's draft value (an Immutable<C> or the
//index'th element of an ImmutableVector<C>)
using SlotAccessor = ImmutableBase& (*)(void* object, int index);

//One level of the "path back up". Lives on the call stack of update() /
//forEach() / visit(), linked to the previous one through a thread-local top.
struct Frame
{
    Frame* previous = nullptr;
    Frame* parent = nullptr; //Frame whose value holds our slot, null for roots
    const ImmutableBase* root = nullptr;
    std::uint64_t editId = 0;
    const TypeOps* ops = nullptr;

    NodeBase* originalNode = nullptr;
    const void* original = nullptr;

    //Root frames own their draft until commit, child drafts live in the slot
    NodeBase* draftNode = nullptr;
    void* draft = nullptr;

    std::ptrdiff_t offset = 0; //Of the slot object within the parent value
    int index = -1; //Element index for vectors, -1 for an Immutable member
    SlotAccessor slotAt = nullptr;
};

inline thread_local Frame* topFrame = nullptr;
inline std::atomic<std::uint64_t> editCounter {0};

inline std::uint64_t newEditId() noexcept
{
    return editCounter.fetch_add(1, std::memory_order_relaxed) + 1;
}

class FrameScope
{
public:
    explicit FrameScope(const Frame& init) noexcept
        : frame(init)
    {
        frame.previous = topFrame;
        topFrame = &frame;
    }

    ~FrameScope()
    {
        topFrame = frame.previous;

        if (frame.parent == nullptr)
            release(frame.draftNode);
    }

    FrameScope(const FrameScope&) = delete;
    FrameScope& operator=(const FrameScope&) = delete;

    Frame frame;
};

inline bool isWithin(const void* begin,
                     std::size_t size,
                     const void* object,
                     std::size_t objectSize) noexcept
{
    if (begin == nullptr)
        return false;

    auto start = reinterpret_cast<std::uintptr_t>(begin);
    auto address = reinterpret_cast<std::uintptr_t>(object);
    return address >= start && address + objectSize <= start + size;
}

inline bool isInOriginal(const Frame& frame, const void* object, std::size_t size)
{
    return isWithin(frame.original, frame.ops->size, object, size);
}

inline bool isInDraft(const Frame& frame, const void* object, std::size_t size)
{
    return isWithin(frame.draft, frame.ops->size, object, size);
}

//Innermost frame whose original or draft value contains the object
inline Frame* findContaining(const void* object, std::size_t size) noexcept
{
    for (auto* frame = topFrame; frame != nullptr; frame = frame->previous)
    {
        if (isInOriginal(*frame, object, size) || isInDraft(*frame, object, size))
            return frame;
    }

    return nullptr;
}

inline std::ptrdiff_t offsetWithin(const Frame& frame, const void* object)
{
    auto base = isInDraft(frame, object, 1) ? frame.draft : frame.original;
    return static_cast<std::ptrdiff_t>(reinterpret_cast<std::uintptr_t>(object)
                                       - reinterpret_cast<std::uintptr_t>(base));
}

//Edit id of the draft containing this object, or 0 when it isn't in one
inline std::uint64_t editIdOwning(const void* object, std::size_t size) noexcept
{
    for (auto* frame = topFrame; frame != nullptr; frame = frame->previous)
    {
        if (isInDraft(*frame, object, size))
            return frame->editId;
    }

    return 0;
}

inline bool isRootOfActiveEdit(const ImmutableBase* handle) noexcept
{
    for (auto* frame = topFrame; frame != nullptr; frame = frame->previous)
    {
        if (frame->root == handle)
            return true;
    }

    return false;
}

//True if a forEach is currently inside a callback for an element of this
//(draft) vector
inline bool isBeingIterated(const void* vector) noexcept
{
    for (auto* frame = topFrame; frame != nullptr; frame = frame->previous)
    {
        auto* parent = frame->parent;

        if (frame->index >= 0 && parent != nullptr && parent->draft != nullptr
            && static_cast<char*>(parent->draft) + frame->offset == vector)
            return true;
    }

    return false;
}

//Makes sure the frame has a draft this edit may mutate: materializes the
//parent chain first, then clones our node into the parent draft's slot
inline void materialize(Frame& frame)
{
    if (frame.draft != nullptr)
        return;

    if (frame.parent == nullptr)
    {
        auto* copy = frame.originalNode != nullptr ? frame.originalNode->clone()
                                                   : frame.ops->createDefault();
        copy->owner = frame.editId;
        frame.draftNode = copy;
        frame.draft = frame.ops->valueOf(copy);
        return;
    }

    materialize(*frame.parent);

    auto* object = static_cast<char*>(frame.parent->draft) + frame.offset;
    auto& slot = frame.slotAt(object, frame.index);
    auto* current = Access::get(slot);
    auto drafted = current != nullptr && current->owner == frame.editId;

    //The slot must still hold the node this frame was pushed for, or the draft
    //made from it earlier in this edit. Anything else means the parent's
    //structure changed underneath the frame.
    EA_IMMUTABLE_ASSERT(drafted || current == frame.originalNode);

    //A node only this draft references (e.g. one added during the edit) can
    //be mutated in place; originals are always shared with the old tree
    if (!drafted && (current == nullptr || !isUnique(current)))
    {
        current = current != nullptr ? current->clone() : frame.ops->createDefault();
        current->owner = frame.editId;
        Access::adopt(slot, current);
    }

    frame.draftNode = current;
    frame.draft = frame.ops->valueOf(current);
}

//Eager copy used by non-const element access on a draft (or local) vector
inline void*
    makeMutable(ImmutableBase& slot, std::uint64_t editId, const TypeOps& ops)
{
    auto* current = Access::get(slot);
    auto drafted = current != nullptr && editId != 0 && current->owner == editId;

    if (!drafted && (current == nullptr || !isUnique(current)))
    {
        current = current != nullptr ? current->clone() : ops.createDefault();
        current->owner = editId;
        Access::adopt(slot, current);
    }

    return ops.valueOf(current);
}

template <typename T>
const T& defaultValue()
{
    static const T instance {};
    return instance;
}
} // namespace ImmutableDetail

//A refcounted handle to an immutable T. Copying shares the node; the value can
//only change through update(). Default-constructed handles hold no node and
//read as a shared default-constructed T (no allocation, so recursive types are
//fine). Only construction from a value needs T to be complete.
template <typename T>
class Immutable : public ImmutableBase
{
public:
    using ValueType = T;

    Immutable() = default;

    Immutable(const T& value)
        : ImmutableBase(new ImmutableDetail::Node<T>(std::in_place, value))
    {
    }

    Immutable(T&& value)
        : ImmutableBase(
              new ImmutableDetail::Node<T>(std::in_place, std::move(value)))
    {
    }

    template <typename... Args>
    explicit Immutable(std::in_place_t, Args&&... args)
        : ImmutableBase(new ImmutableDetail::Node<T>(std::in_place,
                                                     std::forward<Args>(args)...))
    {
    }

    const T& get() const
    {
        if (node == nullptr)
            return ImmutableDetail::defaultValue<T>();

        return static_cast<const ImmutableDetail::Node<T>*>(node)->value;
    }

    const T& operator*() const { return get(); }
    const T* operator->() const { return &get(); }

    //Runs fn(const T&) on the current value as a new edit. Nodes written via
    //write() are path-copied; the result becomes this handle's value when fn
    //returns. If fn throws, nothing changes.
    template <typename Fn>
    void update(Fn&& fn)
    {
        using namespace ImmutableDetail;

        EA_IMMUTABLE_ASSERT(!isRootOfActiveEdit(this));

        auto init = Frame();
        init.root = this;
        init.editId = newEditId();
        init.ops = &typeOps<T>;
        init.originalNode = node;
        init.original = &get();

        auto scope = FrameScope(init);
        fn(get());

        if (scope.frame.draftNode != nullptr)
            adopt(std::exchange(scope.frame.draftNode, nullptr));
    }
};

//A vector of Immutable<T> nodes that reads as a vector of const T.
//Structural operations and non-const operator[] are only reachable through a
//draft (from write()) or a local value that is still being built.
template <typename T>
class ImmutableVector
{
public:
    using Handle = Immutable<T>;

    class ConstIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        ConstIterator() = default;
        explicit ConstIterator(const Handle* handleToUse)
            : handle(handleToUse)
        {
        }

        const T& operator*() const { return handle->get(); }
        const T* operator->() const { return &handle->get(); }

        ConstIterator& operator++()
        {
            ++handle;
            return *this;
        }

        ConstIterator operator++(int)
        {
            auto copy = *this;
            ++handle;
            return copy;
        }

        bool operator==(const ConstIterator& other) const = default;

    private:
        const Handle* handle = nullptr;
    };

    ImmutableVector() = default;

    ImmutableVector(std::initializer_list<T> items)
    {
        for (auto& item: items)
            handles.add(Handle(item));
    }

    int size() const noexcept { return handles.size(); }
    bool isEmpty() const noexcept { return handles.empty(); }

    const T& operator[](int index) const { return handles[index].get(); }

    //Eagerly copies the element (unless this edit already owns it) so a
    //targeted edit like write(node).children[0].gain = 1.f works
    T& operator[](int index)
    {
        auto editId = ImmutableDetail::editIdOwning(this, sizeof(*this));
        auto* value = ImmutableDetail::makeMutable(
            handles[index], editId, ImmutableDetail::typeOps<T>);
        return *static_cast<T*>(value);
    }

    const T& front() const { return handles.front().get(); }
    const T& back() const { return handles.back().get(); }

    const Handle& getHandle(int index) const { return handles[index]; }
    const Vector<Handle>& getHandles() const noexcept { return handles; }

    //True if the element at index is the same node in both vectors
    bool isSameAs(int index, const ImmutableVector& other) const
    {
        return handles[index].isSameAs(other.handles[index]);
    }

    //True if both hold the same nodes in the same order
    bool isSameAs(const ImmutableVector& other) const
    {
        if (size() != other.size())
            return false;

        for (int index = 0; index < size(); ++index)
        {
            if (!isSameAs(index, other))
                return false;
        }

        return true;
    }

    ConstIterator begin() const { return ConstIterator(handles.data()); }
    ConstIterator end() const { return ConstIterator(handles.data() + size()); }

    T& add(const T& value) { return insert(size(), value); }
    T& add(T&& value) { return insert(size(), std::move(value)); }

    //Adds an existing node, sharing it instead of copying
    void addShared(const Handle& handle)
    {
        checkNotIterated();
        handles.add(handle);
    }

    T& insert(int index, const T& value)
    {
        return insertHandle(index, Handle(value));
    }

    T& insert(int index, T&& value)
    {
        return insertHandle(index, Handle(std::move(value)));
    }

    void removeAt(int index)
    {
        checkNotIterated();
        handles.removeAt(index);
    }

    void clear()
    {
        checkNotIterated();
        handles.clear();
    }

private:
    T& insertHandle(int index, Handle&& handle)
    {
        checkNotIterated();
        auto& inserted = handles.insertAt(index, std::move(handle));

        //A freshly created node is referenced only by this vector
        return *static_cast<T*>(
            ImmutableDetail::nodeValue<T>(ImmutableDetail::Access::get(inserted)));
    }

    void checkNotIterated() const
    {
        EA_IMMUTABLE_ASSERT(!ImmutableDetail::isBeingIterated(this));
    }

    Vector<Handle> handles;

    friend struct ImmutableDetail::Access;
};

namespace ImmutableDetail
{
template <typename C>
ImmutableBase& handleSlot(void* object, int)
{
    return *static_cast<Immutable<C>*>(object);
}

template <typename C>
ImmutableBase& vectorSlot(void* object, int index)
{
    return Access::handles(*static_cast<ImmutableVector<C>*>(object))[index];
}

template <typename C>
Frame makeChildFrame(Frame& parent,
                     std::ptrdiff_t offset,
                     int index,
                     const Immutable<C>& handle,
                     SlotAccessor slotAt)
{
    auto frame = Frame();
    frame.parent = &parent;
    frame.editId = parent.editId;
    frame.ops = &typeOps<C>;
    frame.originalNode = Access::get(handle);
    frame.original = &handle.get();
    frame.offset = offset;
    frame.index = index;
    frame.slotAt = slotAt;
    return frame;
}
} // namespace ImmutableDetail

//Visits every element of a vector living inside a node being edited, pushing
//a frame per element so write() can path-copy it. fn takes (const C&) or
//(const C&, int index).
template <typename C, typename Fn>
void forEach(const ImmutableVector<C>& vector, Fn&& fn)
{
    using namespace ImmutableDetail;

    auto* owner = findContaining(&vector, sizeof(vector));
    EA_IMMUTABLE_ASSERT(owner != nullptr);

    auto offset = offsetWithin(*owner, &vector);

    for (int index = 0; index < vector.size(); ++index)
    {
        auto& handle = vector.getHandle(index);
        auto scope = FrameScope(
            makeChildFrame(*owner, offset, index, handle, &vectorSlot<C>));

        if constexpr (std::is_invocable_v<Fn&, const C&, int>)
            fn(handle.get(), index);
        else
            fn(handle.get());
    }
}

//Descends into an Immutable<C> member of a node being edited
template <typename C, typename Fn>
void visit(const Immutable<C>& handle, Fn&& fn)
{
    using namespace ImmutableDetail;

    auto* owner = findContaining(&handle, sizeof(handle));
    EA_IMMUTABLE_ASSERT(owner != nullptr);

    auto offset = offsetWithin(*owner, &handle);
    auto scope =
        FrameScope(makeChildFrame(*owner, offset, -1, handle, &handleSlot<C>));

    fn(handle.get());
}

//Returns the mutable draft of a value reached through update/forEach/visit
//(or of any object inline inside it), copying it and its ancestors at most
//once per edit. See the stale-read rule at the top of this file.
template <typename T>
T& write(const T& value)
{
    using namespace ImmutableDetail;

    auto* frame = findContaining(&value, sizeof(T));
    EA_IMMUTABLE_ASSERT(frame != nullptr);

    auto offset = offsetWithin(*frame, &value);
    materialize(*frame);

    return *static_cast<T*>(
        static_cast<void*>(static_cast<char*>(frame->draft) + offset));
}
} // namespace EA
