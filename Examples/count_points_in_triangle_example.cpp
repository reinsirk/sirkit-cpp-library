// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Count_Points_In_Triangle.cpp"
#include <cassert>
#include <vector>

int main()
{
    using Point = sirkit::CountPointsInTriangle::Point;
    const std::vector<Point> vertices{{0, 0}, {4, 0}, {0, 4}};
    const std::vector<Point> points{{1, 1}, {2, 2}, {0, 0}, {3, 3}};
    const sirkit::CountPointsInTriangle triangles(vertices, points);
    assert(triangles.count(0, 1, 2) == 1);
    assert(triangles.count(2, 1, 0) == 1);
}
