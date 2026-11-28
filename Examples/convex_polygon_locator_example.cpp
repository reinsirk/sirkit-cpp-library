// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Convex_Polygon_Locator.cpp"

#include <cassert>
#include <vector>

int main()
{
    using sirkit::geo::Location;
    using sirkit::geo::PointI;
    const std::vector<PointI> polygon{{0, 0}, {4, 0}, {4, 3}, {0, 3}};
    const sirkit::geo::ConvexPolygonLocator locator(polygon);
    assert(locator.locate({2, 1}) == Location::Inside);
    assert(locator.locate({4, 1}) == Location::Boundary);
    assert(locator.locate({5, 1}) == Location::Outside);
}
