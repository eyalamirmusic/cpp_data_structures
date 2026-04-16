#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ValueWrapper/Constructed.h>

using namespace nano;

auto constructedDefaultsInvalid = test("Constructed.defaults_to_invalid") = []
{
    auto c = EA::Constructed<int>();
    check(!c.isValid());
};

auto constructedCreate = test("Constructed.create_makes_valid") = []
{
    auto c = EA::Constructed<int>();
    c.create(42);
    check(c.isValid());
    check(*c == 42);
};

auto constructedDestroy = test("Constructed.destroy_resets_active") = []
{
    auto c = EA::Constructed<int>();
    c.create(7);
    c.destroy();
    check(!c.isValid());
};

auto constructedAssignValue = test("Constructed.assign_creates_and_updates") = []
{
    auto c = EA::Constructed<int>();
    c = 3;
    check(c.isValid());
    check(*c == 3);
    c = 5;
    check(*c == 5);
};

namespace
{
struct RefCounted
{
    explicit RefCounted(int& counter)
        : count(counter)
    {
        ++count;
    }
    ~RefCounted() { --count; }
    int& count;
};
} // namespace

auto constructedLifecycle = test("Constructed.calls_ctor_and_dtor") = []
{
    auto counter = 0;
    {
        auto c = EA::Constructed<RefCounted>();
        check(counter == 0);
        c.create(counter);
        check(counter == 1);
        c.destroy();
        check(counter == 0);
    }
    check(counter == 0);
};

auto constructedDestructorCleansUp =
    test("Constructed.destructor_cleans_up_active_object") = []
{
    auto counter = 0;
    {
        auto c = EA::Constructed<RefCounted>();
        c.create(counter);
        check(counter == 1);
    }
    check(counter == 0);
};
