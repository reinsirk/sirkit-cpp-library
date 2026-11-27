// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Minimum_Enclosing_Circle.cpp"
#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::MinimumEnclosingCircle::Point> points{{0, 0}, {1, 0}, {2, 0}};
    const auto circle = sirkit::minimum_enclosing_circle(points);
    assert(circle && circle->contains(points[1]));
    assert(circle->on_boundary(points[0]) && !circle->on_boundary(points[1]));
    assert(circle->center().first == 1 && circle->radius_squared() == 1);
}
