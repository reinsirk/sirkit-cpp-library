// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Polygon_Chord_Moments.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array<sirkit::PolygonChordMoments::Point, 4> square{
        {{0, 0}, {6, 0}, {6, 6}, {0, 6}}};
    const sirkit::PolygonChordMoments polygon(square);
    assert(polygon.total().signed_area2 == 72);
    assert(polygon.total().centroid == (std::pair{3., 3.}));
    assert(polygon.slice(0, 2).centroid == (std::pair{4., 2.}));
    assert(!polygon.slice(0, 1).centroid);
}
