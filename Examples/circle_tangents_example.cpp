// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Circle_Tangents.cpp"
#include <cassert>

int main()
{
    const sirkit::geo::Circle a{{0, 0}, 1}, b{{5, 0}, 2};
    const auto tangents = sirkit::geo::circle_tangents(a, b);
    assert(tangents && tangents->size() == 4);
    const auto from_point = sirkit::geo::point_circle_tangents(a, {2, 0});
    assert(from_point.size() == 2);
}
