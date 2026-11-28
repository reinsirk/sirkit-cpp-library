// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n = 0, m = 0;
    verify_in >> n >> m;
    std::vector<std::pair<std::int64_t, std::int64_t>> a(n), b(m);
    for (auto &[x, y] : a)
    {
        verify_in >> x >> y;
    }
    for (auto &[x, y] : b)
    {
        verify_in >> x >> y;
    }
    const auto common = sirkit::convex_polygon_intersection(a, b);
    double area = 0;
    for (std::size_t i = 2; i < common.size(); ++i)
    {
        const auto [ax, ay] = common[0];
        const auto [bx, by] = common[i - 1];
        const auto [cx, cy] = common[i];
        area += (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
    }
    verify_out.fixed(12) << std::abs(area) / 2 << '\n';
}
