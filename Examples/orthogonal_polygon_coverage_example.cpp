// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Orthogonal_Polygon_Coverage.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    using namespace sirkit::geo;
    const std::vector<OrthogonalPolygon> polygons{
        {{0, 0}, {3, 0}, {3, 2}, {0, 2}},
        {{2, 1}, {2, 4}, {4, 4}, {4, 1}},
    };
    const std::vector<OrthogonalPoint> cells{{0, 0}, {2, 1}, {3, 2}, {4, 1}};
    assert((orthogonal_polygon_coverage(polygons, cells) == std::vector<std::int64_t>{1, 2, 1, 0}));
}
