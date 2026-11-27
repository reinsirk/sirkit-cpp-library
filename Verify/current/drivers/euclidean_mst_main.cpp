// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<std::pair<std::int64_t, std::int64_t>> points(n);
    for (auto &[x, y] : points)
    {
        verify_in >> x >> y;
    }
    for (const auto &edge : sirkit::euclidean_mst(points))
    {
        verify_out << edge.from << ' ' << edge.to << '\n';
    }
}
