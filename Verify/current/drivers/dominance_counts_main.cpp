// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <numeric>
#include <utility>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<std::pair<int, int>> points(n);
    for (auto &point : points)
    {
        verify_in >> point.first;
    }
    for (auto &point : points)
    {
        verify_in >> point.second;
    }
    const auto counts = sirkit::dominance_counts(points, false, false, std::greater<>{});
    verify_out << std::accumulate(counts.begin(), counts.end(), std::uint64_t{0}) << '\n';
}
