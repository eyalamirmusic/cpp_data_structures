#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>

using namespace nano;

namespace
{
struct OwnedBase : EA::CloneableInterface<OwnedBase>
{
    virtual int value() const = 0;
};

struct OwnedDerivedA : EA::Cloneable<OwnedBase, OwnedDerivedA>
{
    explicit OwnedDerivedA(int v)
        : data(v)
    {
    }
    OwnedDerivedA(const OwnedDerivedA&) = default;

    int value() const override { return data; }
    int data = 0;
};

struct OwnedItem
{
    explicit OwnedItem(int v)
        : value(v)
    {
    }

    bool operator==(int other) const { return value == other; }
    bool operator==(const OwnedItem& other) const { return value == other.value; }

    int value = 0;
};
} // namespace

auto ownedVectorCreateNew = test("OwnedVector.createNew_adds_owned_object") = []
{
    auto v = EA::OwnedVector<OwnedItem>();
    v.createNew(5);
    v.createNew(10);
    check(v.size() == 2);
    check(v[0]->value == 5);
    check(v[1]->value == 10);
};

auto ownedVectorContains = test("OwnedVector.contains_uses_value_equality") = []
{
    auto v = EA::OwnedVector<OwnedItem>();
    v.createNew(1);
    v.createNew(2);
    check(v.contains(2));
    check(!v.contains(99));
};

auto ownedVectorGetIndexOfElement = test("OwnedVector.getIndexOfElement") = []
{
    auto v = EA::OwnedVector<OwnedItem>();
    v.createNew(10);
    v.createNew(20);
    v.createNew(30);
    check(v.getIndexOfElement(20) == 1);
    check(v.getIndexOfElement(999) == -1);
};

auto ownedVectorGetIndexOfItem = test("OwnedVector.getIndexOfItem_by_pointer") = []
{
    auto v = EA::OwnedVector<OwnedItem>();
    auto& a = v.createNew(10);
    auto& b = v.createNew(20);
    check(v.getIndexOfItem(&a) == 0);
    check(v.getIndexOfItem(&b) == 1);
};

auto ownedVectorRemoveItem = test("OwnedVector.removeItem_by_reference") = []
{
    auto v = EA::OwnedVector<OwnedItem>();
    auto& a = v.createNew(10);
    v.createNew(20);
    v.removeItem(a);
    check(v.size() == 1);
    check(v[0]->value == 20);
};

auto ownedVectorRemoveAllMatches = test("OwnedVector.removeAllMatches_value") = []
{
    auto v = EA::OwnedVector<OwnedItem>();
    v.createNew(1);
    v.createNew(2);
    v.createNew(1);
    v.createNew(3);
    v.removeAllMatches(1);
    check(v.size() == 2);
};

auto ownedVectorAddElementIfNotThere = test("OwnedVector.addElementIfNotThere") = []
{
    auto v = EA::OwnedVector<OwnedItem>();
    v.createNew(1);
    check(!v.addElementIfNotThere(1));
    check(v.addElementIfNotThere(2));
    check(v.size() == 2);
};

auto ownedVectorCreateDerived =
    test("OwnedVector.createDerived_stores_derived_instance") = []
{
    auto v = EA::OwnedVector<OwnedBase>();
    auto& derived = v.createDerived<OwnedDerivedA>(42);
    check(v.size() == 1);
    check(derived.value() == 42);
    check(v[0]->value() == 42);
};
