#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Utilities/MapUtilities.h>
#include <map>

using namespace nano;

auto mapUtilsContains = test("MapUtils.contains") = []
{
    auto m = std::map<int, int> {{1, 10}, {2, 20}};
    check(EA::MapUtils::contains(m, 1));
    check(!EA::MapUtils::contains(m, 99));
};

auto mapUtilsGet = test("MapUtils.get_returns_pointer_or_null") = []
{
    auto m = std::map<int, int> {{1, 10}};
    auto* p = EA::MapUtils::get(m, 1);
    check(p != nullptr);
    check(*p == 10);
    check(EA::MapUtils::get(m, 99) == nullptr);
};

auto mapUtilsGetDefault = test("MapUtils.get_with_default") = []
{
    auto m = std::map<int, int> {{1, 10}};
    check(EA::MapUtils::get(m, 1, -1) == 10);
    check(EA::MapUtils::get(m, 99, -1) == -1);
};
