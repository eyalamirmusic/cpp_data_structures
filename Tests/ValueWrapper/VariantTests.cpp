#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ValueWrapper/Variant.h>
#include <Helpers/OperationTracker.h>

#include <string>
#include <utility>

using namespace nano;

namespace
{
using EA::TestHelpers::OperationTracker;
using IntOrString = EA::Variant<int, std::string>;
using Tracked = EA::Variant<int, OperationTracker>;

struct First
{
    int value = 5;
};

struct Second
{
    std::string text = "Hello";
};

struct Third
{
    double amount = 2.5;
};

using Three = EA::Variant<First, Second, Third>;

template <typename... Fs>
struct Overloaded : Fs...
{
    using Fs::operator()...;
};

template <typename... Fs>
Overloaded(Fs...) -> Overloaded<Fs...>;

struct NameVisitor
{
    std::string operator()(const First&) const { return "First"; }
    std::string operator()(const Second&) const { return "Second"; }
    std::string operator()(const Third&) const { return "Third"; }
};

constexpr int constexprVisit()
{
    auto v = EA::Variant<int, double>(3.0);
    auto result = v.visit([](auto x) { return static_cast<int>(x) * 2; });

    v = 10;
    result += v.visit([](auto x) { return static_cast<int>(x); });
    return result;
}

static_assert(constexprVisit() == 16);
static_assert(EA::Variant<int, double>(1.0).holds<double>());
static_assert(EA::Variant<int, double>(1).index() == 0);
} // namespace

auto variantDefaultConstructsFirst = test("Variant.default_constructs_first") = []
{
    auto v = IntOrString();
    check(v.index() == 0);
    check(v.holds<int>());
    check(*v.get<int>() == 0);
};

auto variantConstructFromEach = test("Variant.construct_from_each_alternative") = []
{
    auto a = IntOrString(42);
    check(a.index() == 0);
    check(a.holds<int>());
    check(!a.holds<std::string>());
    check(*a.get<int>() == 42);

    auto b = IntOrString(std::string("text"));
    check(b.index() == 1);
    check(b.holds<std::string>());
    check(!b.holds<int>());
    check(*b.get<std::string>() == "text");

    auto c = Three(Third {});
    check(c.index() == 2);
    check(c.get<Third>()->amount == 2.5);
};

auto variantInPlaceConstruct = test("Variant.in_place_construct") = []
{
    auto v = IntOrString(std::in_place_type<std::string>, 3, 'x');
    check(*v.get<std::string>() == "xxx");

    auto w = IntOrString(std::in_place_index<0>, 9);
    check(*w.get<int>() == 9);
};

auto variantGetWrongType = test("Variant.get_wrong_type_returns_null") = []
{
    auto v = IntOrString(1);
    check(v.get<std::string>() == nullptr);
    check(v.get<int>() != nullptr);

    const auto& cv = v;
    check(cv.get<std::string>() == nullptr);
    check(cv.get<int>() != nullptr);
};

auto variantGetMutates = test("Variant.get_allows_mutation") = []
{
    auto v = IntOrString(1);
    *v.get<int>() = 7;
    check(*v.get<int>() == 7);

    const auto& cv = v;
    const int* p = cv.get<int>();
    check(*p == 7);
};

auto variantCopy = test("Variant.copy_construct_and_assign") = []
{
    auto a = IntOrString(std::string("copy"));
    auto b = a;
    check(b.holds<std::string>());
    check(*b.get<std::string>() == "copy");
    check(*a.get<std::string>() == "copy");

    auto c = IntOrString(3);
    c = a;
    check(c.holds<std::string>());
    check(*c.get<std::string>() == "copy");
};

auto variantMove = test("Variant.move_construct_and_assign") = []
{
    auto a = IntOrString(std::string("a fairly long string to avoid SSO"));
    auto b = std::move(a);
    check(b.holds<std::string>());
    check(*b.get<std::string>() == "a fairly long string to avoid SSO");

    auto c = IntOrString(3);
    c = std::move(b);
    check(c.holds<std::string>());
    check(*c.get<std::string>() == "a fairly long string to avoid SSO");
};

auto variantAssignAcross = test("Variant.assign_across_alternatives") = []
{
    auto v = IntOrString(5);
    v = std::string("now a string");
    check(v.index() == 1);
    check(*v.get<std::string>() == "now a string");

    v = 12;
    check(v.index() == 0);
    check(*v.get<int>() == 12);
    check(v.get<std::string>() == nullptr);
};

auto variantEmplace = test("Variant.emplace") = []
{
    auto v = IntOrString(5);
    v.emplace<std::string>("emplaced");
    check(v.holds<std::string>());
    check(*v.get<std::string>() == "emplaced");

    v.emplace<0>(99);
    check(*v.get<int>() == 99);
};

auto variantVisitReturnsValue = test("Variant.visit_returns_value") = []
{
    auto v = Three(First {});
    auto size = [](auto& obj) { return static_cast<int>(sizeof(obj)); };
    check(v.visit(size) == static_cast<int>(sizeof(First)));

    v = Second {};
    check(v.visit(size) == static_cast<int>(sizeof(Second)));

    v = Third {};
    check(v.visit(size) == static_cast<int>(sizeof(Third)));
};

auto variantVisitEveryIndex = test("Variant.visit_dispatches_every_index") = []
{
    auto names = NameVisitor();

    check(Three(First {}).visit(names) == "First");
    check(Three(Second {}).visit(names) == "Second");
    check(Three(Third {}).visit(names) == "Third");
};

auto variantVisitMutates = test("Variant.visit_non_const_mutates") = []
{
    auto v = IntOrString(1);
    v.visit(Overloaded {[](int& i) { i += 10; }, [](std::string& s) { s += "!"; }});
    check(*v.get<int>() == 11);

    v = std::string("hi");
    v.visit(Overloaded {[](int& i) { i += 10; }, [](std::string& s) { s += "!"; }});
    check(*v.get<std::string>() == "hi!");
};

auto variantVisitConst = test("Variant.visit_const") = []
{
    const auto v = IntOrString(std::string("abc"));

    auto isConst =
        v.visit([](auto& obj)
                { return std::is_const_v<std::remove_reference_t<decltype(obj)>>; });
    check(isConst);

    auto length = v.visit(Overloaded {[](const int&) { return 0; },
                                      [](const std::string& s)
                                      { return static_cast<int>(s.size()); }});
    check(length == 3);
};

auto variantVisitOverloaded = test("Variant.visit_overloaded_lambdas") = []
{
    auto describe = Overloaded {[](int i) { return std::to_string(i); },
                                [](const std::string& s) { return "str:" + s; }};

    check(IntOrString(4).visit(describe) == "4");
    check(IntOrString(std::string("x")).visit(describe) == "str:x");
};

auto variantSingleAlternative = test("Variant.single_alternative") = []
{
    auto v = EA::Variant<int>(8);
    check(v.index() == 0);
    check(v.visit([](int& i) { return i * 2; }) == 16);

    v.visit([](int& i) { i = 3; });
    check(*v.get<int>() == 3);
};

auto variantFreeVisitMultiple = test("Variant.free_visit_multiple_variants") = []
{
    auto a = IntOrString(2);
    auto b = EA::Variant<int, double>(0.5);

    auto combine = Overloaded {
        [](int x, int y) { return std::string("ii") + std::to_string(x + y); },
        [](int, double) { return std::string("id"); },
        [](const std::string&, int) { return std::string("si"); },
        [](const std::string&, double) { return std::string("sd"); }};

    check(EA::Rollbear::visit(combine, a, b) == "id");

    b = 3;
    check(EA::Rollbear::visit(combine, a, b) == "ii5");

    a = std::string("s");
    check(EA::Rollbear::visit(combine, a, b) == "si");

    b = 1.0;
    check(EA::Rollbear::visit(combine, a, b) == "sd");
};

auto variantFreeVisitPlainValues = test("Variant.free_visit_plain_values") = []
{
    auto plain = 4;
    auto v = IntOrString(std::string("abc"));

    auto result =
        EA::Rollbear::visit(Overloaded {[](int x, int y) { return x + y; },
                                        [](int x, const std::string& s)
                                        { return x + static_cast<int>(s.size()); }},
                            plain,
                            v);
    check(result == 7);

    check(EA::Rollbear::visit([](int x) { return x + 1; }, plain) == 5);
};

auto variantDestroysAlternative = test("Variant.destroys_active_alternative") = []
{
    OperationTracker::reset();
    {
        auto v = Tracked(std::in_place_type<OperationTracker>, 7);
        check(OperationTracker::counters.live() == 1);
        check(v.get<OperationTracker>()->getValue() == 7);
    }
    check(OperationTracker::counters.destructions == 1);
    check(OperationTracker::counters.live() == 0);
};

auto variantSwitchDestroys = test("Variant.switching_alternative_destroys_old") = []
{
    OperationTracker::reset();
    auto v = Tracked(std::in_place_type<OperationTracker>, 1);
    check(OperationTracker::counters.live() == 1);

    v = 5;
    check(OperationTracker::counters.live() == 0);
    check(OperationTracker::counters.destructions == 1);
    check(v.holds<int>());

    v.emplace<OperationTracker>(2);
    check(OperationTracker::counters.live() == 1);
};

auto variantCopyTracked = test("Variant.copy_and_move_tracked_alternative") = []
{
    OperationTracker::reset();
    {
        auto a = Tracked(std::in_place_type<OperationTracker>, 3);
        auto b = a;
        check(OperationTracker::counters.copyConstructions == 1);
        check(b.get<OperationTracker>()->getValue() == 3);

        auto c = std::move(a);
        check(OperationTracker::counters.moveConstructions == 1);
        check(c.get<OperationTracker>()->getValue() == 3);

        b = c;
        check(OperationTracker::counters.copyAssignments == 1);

        check(OperationTracker::counters.live() == 3);
    }
    check(OperationTracker::counters.live() == 0);
};

auto variantVisitNoCopies = test("Variant.visit_passes_by_reference") = []
{
    OperationTracker::reset();
    auto v = Tracked(std::in_place_type<OperationTracker>, 4);

    auto value =
        v.visit(Overloaded {[](int i) { return i; },
                            [](const OperationTracker& t) { return t.getValue(); }});
    check(value == 4);
    check(OperationTracker::counters.totalConstructions() == 1);
};

auto variantStdInterop = test("Variant.std_variant_interop") = []
{
    auto v = IntOrString(std::string("std"));
    check(std::holds_alternative<std::string>(v));
    check(std::get<std::string>(v) == "std");
    check(IntOrString(1) == IntOrString(1));
    check(IntOrString(1) != IntOrString(2));
};
