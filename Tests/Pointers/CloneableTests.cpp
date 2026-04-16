#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Pointers/Cloneable.h>

using namespace nano;

namespace
{
struct CloneableBase : EA::CloneableInterface<CloneableBase>
{
    virtual int value() const = 0;
};

struct CloneableDerived : EA::Cloneable<CloneableBase, CloneableDerived>
{
    explicit CloneableDerived(int v)
        : data(v)
    {
    }
    CloneableDerived(const CloneableDerived&) = default;
    int value() const override { return data; }
    int data = 0;
};
} // namespace

auto cloneableReturnsNewInstance =
    test("Cloneable.clone_returns_new_base_pointer") = []
{
    auto original = CloneableDerived(42);
    auto* cloned = original.clone();
    check(cloned != nullptr);
    check(cloned != static_cast<CloneableBase*>(&original));
    check(cloned->value() == 42);
    delete cloned;
};

auto cloneableCopiesState = test("Cloneable.clone_copies_derived_state") = []
{
    auto original = CloneableDerived(7);
    auto* cloned = original.clone();
    original.data = 999;
    check(cloned->value() == 7);
    delete cloned;
};
