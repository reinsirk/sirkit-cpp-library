// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Delaunay_Triangulation.cpp"
#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::DelaunayPoint> points{{0, 0}, {4, 0}, {1, 3}, {1, 1}};
    const auto edges = sirkit::delaunay_triangulation(points);
    assert(edges.size() == 6);
}
