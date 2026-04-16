#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Pointers/Any.h>

using namespace nano;

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
