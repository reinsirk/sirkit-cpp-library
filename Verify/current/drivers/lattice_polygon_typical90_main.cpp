// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n = 0;
    verify_in >> n;
    using Point = sirkit::geo::Point<std::int64_t, std::int64_t>;
    std::vector<Point> points(n);
    for (auto &[x, y] : points)
    {
        verify_in >> x >> y;
    }
    const auto hull = sirkit::geo::convex_hull(std::move(points));
    std::vector<std::pair<std::int64_t, std::int64_t>> vertices;
    vertices.reserve(hull.size());
    for (const auto &[x, y] : hull)
    {
        vertices.emplace_back(x, y);
    }
    verify_out << sirkit::lattice_polygon_points(vertices).total() - static_cast<std::uint64_t>(n) << '\n';
}
