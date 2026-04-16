#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Utilities/StaticObjects.h>

using namespace nano;

namespace
{
struct StaticThing
{
    int v = 5;
};
} // namespace

auto staticObjectSameRef = test("getStaticObject.returns_same_reference") = []
{
    auto& a = EA::getStaticObject<StaticThing>();
    auto& b = EA::getStaticObject<StaticThing>();
    check(&a == &b);
    check(a.v == 5);
};

auto staticStackObjectSameRef = test("getStaticStackObject.returns_same_ref") = []
{
    auto& a = EA::getStaticStackObject<StaticThing>();
    auto& b = EA::getStaticStackObject<StaticThing>();
    check(&a == &b);
};

auto updateMarkerEquals = test("UpdateMarker.new_markers_are_equal") = []
{
    auto a = EA::UpdateMarker();
    auto b = EA::UpdateMarker();
    check(a == b);
};

auto updateMarkerUpdateDiverges = test("UpdateMarker.update_diverges") = []
{
    auto a = EA::UpdateMarker();
    auto b = EA::UpdateMarker();
    a.update();
    check(!(a == b));
    b.update();
    check(a == b);
};
