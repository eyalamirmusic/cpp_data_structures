#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>
#include <ea_data_structures/Pointers/OwningPointer.h>
#include <memory>

using namespace nano;

namespace
{
struct PointerBase : EA::CloneableInterface<PointerBase>
{
    virtual int value() const = 0;
};

struct PointerDerived : EA::Cloneable<PointerBase, PointerDerived>
{
    explicit PointerDerived(int v)
        : data(v)
    {
    }
    PointerDerived(const PointerDerived&) = default;
    int value() const override { return data; }
    int data = 0;
};
} // namespace

auto owningPtrDefault = test("OwningPointer.defaults_to_null") = []
{
    auto p = EA::OwningPointer<int>();
    check(p == nullptr);
    check(p.get() == nullptr);
};

auto owningPtrCreateAndAccess = test("OwningPointer.create_and_access") = []
{
    auto p = EA::OwningPointer<int>();
    p.create(42);
    check(p != nullptr);
    check(*p == 42);
    check(*p.get() == 42);
};

auto owningPtrMakeOwned = test("OwningPointer.makeOwned_helper") = []
{
    auto p = EA::makeOwned<int>(7);
    check(p != nullptr);
    check(*p == 7);
};

auto owningPtrReset = test("OwningPointer.reset_deletes_previous") = []
{
    auto p = EA::makeOwned<int>(1);
    p.reset();
    check(p == nullptr);
};

auto owningPtrRelease = test("OwningPointer.release_transfers_ownership") = []
{
    auto p = EA::makeOwned<int>(3);
    auto* raw = p.release();
    check(p == nullptr);
    check(*raw == 3);
    delete raw;
};

auto owningPtrMove = test("OwningPointer.move_transfers") = []
{
    auto source = EA::makeOwned<int>(9);
    auto dest = std::move(source);
    check(source == nullptr);
    check(dest != nullptr);
    check(*dest == 9);
};

auto owningPtrFromUniquePtr = test("OwningPointer.adopts_unique_ptr") = []
{
    auto u = std::make_unique<int>(5);
    auto p = EA::OwningPointer<int>(std::move(u));
    check(u == nullptr);
    check(*p == 5);
};

auto owningPtrGetOrCreate = test("OwningPointer.getOrCreate_only_creates_once") = []
{
    auto p = EA::OwningPointer<int>();
    auto& a = p.getOrCreate(7);
    auto& b = p.getOrCreate(99);
    check(&a == &b);
    check(a == 7);
};

auto owningPtrClone = test("OwningPointer.copy_uses_clone") = []
{
    auto original = EA::OwningPointer<PointerBase>();
    original.create<PointerDerived>(42);

    auto copy = original;
    check(copy != nullptr);
    check(copy.get() != original.get());
    check(copy->value() == 42);
};

auto owningPtrGetAs = test("OwningPointer.getAs_downcasts") = []
{
    auto p = EA::OwningPointer<PointerBase>();
    p.create<PointerDerived>(10);
    auto* derived = p.getAs<PointerDerived>();
    check(derived != nullptr);
    check(derived->data == 10);
};

auto refFromReference = test("Ref.constructs_from_reference") = []
{
    auto x = 42;
    auto r = EA::Ref<int>(x);
    check(r.get() == &x);
    check(*r == 42);
};

auto refFromPointer = test("Ref.constructs_from_pointer") = []
{
    auto x = 7;
    auto r = EA::Ref<int>(&x);
    check(*r == 7);
};

auto refFromOwningPointer = test("Ref.constructs_from_OwningPointer") = []
{
    auto p = EA::makeOwned<int>(5);
    auto r = EA::Ref<int>(p);
    check(*r == 5);
};

auto refOrOwnDefault = test("RefOrOwn.defaults_to_null") = []
{
    auto r = EA::RefOrOwn<int>();
    check(r.get() == nullptr);
};

auto refOrOwnReferences = test("RefOrOwn.references_object") = []
{
    auto x = 5;
    auto r = EA::RefOrOwn<int>(x);
    check(r.get() == &x);
    check(*r == 5);
};

auto refOrOwnCreates = test("RefOrOwn.create_owns_new_object") = []
{
    auto r = EA::RefOrOwn<int>();
    r.create(42);
    check(*r == 42);
};

auto refOrOwnCreateIfNeeded = test("RefOrOwn.createIfNeeded_skips_when_set") = []
{
    auto r = EA::RefOrOwn<int>();
    r.create(1);
    auto* firstPtr = r.get();
    r.createIfNeeded(2);
    check(r.get() == firstPtr);
    check(*r == 1);
};

auto refOrOwnShared = test("RefOrOwn.copy_shares_owned") = []
{
    auto a = EA::RefOrOwn<int>();
    a.create(7);
    auto b = a;
    check(b.get() == a.get());
    check(*b == 7);
};

auto callbackFuncDefault = test("CallbackFunc.default_is_noop") = []
{
    auto cb = EA::CallbackFunc();
    cb();
    check(true);
};

auto callbackFuncInvokes = test("CallbackFunc.invokes_stored_lambda") = []
{
    auto called = false;
    auto cb = EA::CallbackFunc([&] { called = true; });
    cb();
    check(called);
};

auto callbackFuncReassign = test("CallbackFunc.reassignment_replaces") = []
{
    auto cb = EA::CallbackFunc();
    auto counter = 0;
    cb = [&] { ++counter; };
    cb();
    cb();
    check(counter == 2);
};

auto callbackFuncCallMethod = test("CallbackFunc.call_method_works") = []
{
    auto called = false;
    auto cb = EA::CallbackFunc([&] { called = true; });
    cb.call();
    check(called);
};

auto callbackWithArg = test("Callback.returns_value_with_arg") = []
{
    auto cb = EA::Callback<int, int>([](int x) { return x * 2; });
    check(cb(5) == 10);
};

auto callbackWithArgDefault = test("Callback.default_returns_default_value") = []
{
    auto cb = EA::Callback<int, int>();
    check(cb(42) == 0);
};

auto chainFunctionsTest = test("chainFunctions.invokes_in_order") = []
{
    auto order = std::string();
    auto chained = EA::chainFunctions([&] { order += "a"; },
                                      [&] { order += "b"; });
    chained();
    check(order == "ab");
};

auto anyStoreAndGet = test("Any.create_and_get_same_type") = []
{
    struct Foo
    {
        int x = 42;
    };
    auto a = EA::Any();
    auto* p = a.create<Foo>();
    check(p != nullptr);
    check(a.get<Foo>() == p);
    check(a.get<Foo>()->x == 42);
};

auto anyGetWrongTypeReturnsNull = test("Any.get_wrong_type_returns_null") = []
{
    struct Foo
    {
        int x = 1;
    };
    struct Bar
    {
        int y = 2;
    };
    auto a = EA::Any();
    a.create<Foo>();
    check(a.get<Bar>() == nullptr);
};
