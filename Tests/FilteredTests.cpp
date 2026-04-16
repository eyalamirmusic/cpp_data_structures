#include <NanoTest/NanoTest.h>
#include <ea_data_structures/ea_data_structures.h>

using namespace nano;

auto filteredFilter = test("Filtered.filter_populates_indexes") = []
{
    auto v = EA::Vector<int> {1, 2, 3, 4, 5};
    auto f = EA::Utilities::Filtered(v);
    f.filter([](int x) { return x % 2 == 0; });
    check(f.indexes.size() == 2);
    check(f.indexes[0] == 1);
    check(f.indexes[1] == 3);
};

auto filteredForEach = test("Filtered.forEach_visits_matches") = []
{
    auto v = EA::Vector<int> {1, 2, 3, 4, 5};
    auto f = EA::Utilities::Filtered(v);
    f.filter([](int x) { return x > 2; });

    auto sum = 0;
    f.forEach([&](int x) { sum += x; });
    check(sum == 3 + 4 + 5);
};

auto filteredRemoveAll = test("Filtered.removeAll_erases_matches") = []
{
    auto v = EA::Vector<int> {1, 2, 3, 4, 5};
    auto f = EA::Utilities::Filtered(v);
    f.filter([](int x) { return x % 2 == 0; });
    f.removeAll();
    check(v.size() == 3);
    check(v[0] == 1);
    check(v[1] == 3);
    check(v[2] == 5);
};
