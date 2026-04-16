#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Pointers/Cloneable.h>
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
