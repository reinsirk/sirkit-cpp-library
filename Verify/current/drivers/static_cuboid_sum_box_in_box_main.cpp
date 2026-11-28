// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

int main()
{
    int n = 0;
    verify_in >> n;
    std::vector<sirkit::StaticCuboidSumPoint> points;
    std::vector<sirkit::StaticCuboidSumQuery> queries;
    points.reserve(n);
    queries.reserve(n);
    while (n-- > 0)
    {
        std::array<std::int64_t, 3> dimensions{};
        verify_in >> dimensions[0] >> dimensions[1] >> dimensions[2];
        std::ranges::sort(dimensions);
        points.push_back({dimensions[0], dimensions[1], dimensions[2], 1});
        queries.push_back({{}, dimensions});
    }
    const auto counts = sirkit::static_cuboid_sum(points, queries);
    const auto found = std::ranges::any_of(counts, [](std::int64_t value)
                                           {
                                               return value > 0;
                                           });
    verify_out << (found ? "Yes\n" : "No\n");
}
