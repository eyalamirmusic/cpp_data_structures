#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>
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
