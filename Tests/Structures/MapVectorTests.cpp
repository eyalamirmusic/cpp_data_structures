#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/MapVector.h>
#include <string>

using namespace nano;

auto mapVectorInsertViaSubscript = test("MapVector.subscript_inserts") = []
{
    auto map = EA::MapVector<int, std::string>();
    map[1] = "one";
    map[2] = "two";
    check(map.size() == 2);
};

auto mapVectorSubscriptReturnsExisting = test("MapVector.subscript_returns_existing") = []
{
    auto map = EA::MapVector<int, std::string>();
    map[1] = "one";
    auto& existing = map[1];
    check(existing == "one");
    check(map.size() == 1);
};

auto mapVectorGetValue = test("MapVector.getValue") = []
{
    auto map = EA::MapVector<int, std::string>();
    map[7] = "seven";

    auto* v = map.getValue(7);
    check(v != nullptr);
    check(*v == "seven");

    check(map.getValue(999) == nullptr);
};

auto mapVectorRemove = test("MapVector.remove") = []
{
    auto map = EA::MapVector<int, int>();
    map[1] = 100;
    map[2] = 200;
    map.remove(1);
    check(map.size() == 1);
    check(map.getValue(1) == nullptr);
    check(*map.getValue(2) == 200);
};

auto mapVectorEmpty = test("MapVector.empty") = []
{
    auto map = EA::MapVector<int, int>();
    check(map.empty());
    map[1] = 1;
    check(!map.empty());
    map.clear();
    check(map.empty());
};

auto mapVectorEmplace = test("MapVector.emplace_constructs_value") = []
{
    auto map = EA::MapVector<int, int>();
    auto& value = map.emplace(1, 42);
    check(map.size() == 1);
    check(value == 42);
    check(*map.getValue(1) == 42);
};

auto mapVectorSortByKey = test("MapVector.sortByKey") = []
{
    auto map = EA::MapVector<int, int>();
    map[3] = 30;
    map[1] = 10;
    map[2] = 20;
    map.sortByKey();
    check(map.getKey(0) == 1);
    check(map.getKey(1) == 2);
    check(map.getKey(2) == 3);
};

auto mapVectorSortByValue = test("MapVector.sortByValue_default_is_ascending") = []
{
    auto map = EA::MapVector<int, int>();
    map[1] = 300;
    map[2] = 100;
    map[3] = 200;
    map.sortByValue();
    check(map.get(0) == 100);
    check(map.get(1) == 200);
    check(map.get(2) == 300);
};

auto mapVectorSortByValueDescending =
    test("MapVector.sortByValue_forward_false_is_descending") = []
{
    auto map = EA::MapVector<int, int>();
    map[1] = 300;
    map[2] = 100;
    map[3] = 200;
    map.sortByValue(false);
    check(map.get(0) == 300);
    check(map.get(1) == 200);
    check(map.get(2) == 100);
};

auto mapVectorGetKeyByValue = test("MapVector.getKeyByValue") = []
{
    auto map = EA::MapVector<int, int>();
    map[7] = 42;
    map[8] = 99;
    auto* key = map.getKeyByValue(42);
    check(key != nullptr);
    check(*key == 7);
    check(map.getKeyByValue(12345) == nullptr);
};

auto mapVectorHasMatch = test("MapVector.hasMatch_on_value") = []
{
    auto map = EA::MapVector<int, int>();
    map[1] = 10;
    map[2] = 20;
    check(map.hasMatch(10));
    check(!map.hasMatch(999));
};

auto mapVectorReserve = test("MapVector.reserve_preserves_size") = []
{
    auto map = EA::MapVector<int, int>();
    map.reserve(100);
    check(map.empty());
    map[1] = 1;
    check(map.size() == 1);
};
