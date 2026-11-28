// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n;
    verify_in >> n;
    std::vector<sirkit::geo::OrthogonalPolygon> polygons(n);
    for (auto &polygon : polygons)
    {
        int count;
        verify_in >> count;
        polygon.resize(count);
        for (auto &[x, y] : polygon)
        {
            verify_in >> x >> y;
        }
    }
    int q;
    verify_in >> q;
    std::vector<sirkit::geo::OrthogonalPoint> cells(q);
    for (auto &[x, y] : cells)
    {
        verify_in >> x >> y;
    }
    for (const auto count : sirkit::geo::orthogonal_polygon_coverage(polygons, cells))
    {
        verify_out << count << '\n';
    }
}
