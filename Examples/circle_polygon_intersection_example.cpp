// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Circle_Polygon_Intersection.cpp"

#include <array>
#include <cassert>
#include <cmath>
#include <numbers>
#include <utility>

int main()
{
    const std::array<std::pair<double, double>, 4> right_half{
        {{0, -3}, {4, -3}, {4, 3}, {0, 3}}};
    const double area = sirkit::circle_polygon_intersection_area(right_half, 2);
    assert(std::abs(area - 2 * std::numbers::pi) < 1e-12);
}
