#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Pointers/RefOrOwn.h>

using namespace nano;

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
