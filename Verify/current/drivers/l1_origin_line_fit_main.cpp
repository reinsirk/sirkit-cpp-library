// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <utility>
#include <vector>
#include "Verify/current/common/fast_io.hpp"

int main()
{
    int n = 0;
    verify_in >> n;
    std::vector<std::pair<std::int32_t, std::int32_t>> points(n);
    for (auto &point : points)
    {
        verify_in >> point.first;
    }
    for (auto &point : points)
    {
        verify_in >> point.second;
    }
    const auto result = l1_origin_line_fit(points);
    verify_out << result.cost.numerator() << ' ' << result.cost.denominator() << '\n';
}
