// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int tests{};
    verify_in >> tests;
    std::vector<std::pair<std::int32_t, std::int32_t>> points;
    while (tests-- != 0)
    {
        int n{};
        verify_in >> n;
        points.resize(n);
        for (auto &[x, y] : points)
        {
            verify_in >> x >> y;
        }
        const auto answer = furthest_pair(points);
        verify_out << answer->first_index << ' ' << answer->second_index << '\n';
    }
}
