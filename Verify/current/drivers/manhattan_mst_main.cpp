// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<std::pair<std::int64_t, std::int64_t>> points(n);
    for (auto &[x, y] : points)
    {
        verify_in >> x >> y;
    }
    const auto tree = manhattan_mst(points);
    verify_out << tree.cost << '\n';
    for (const auto &[u, v] : tree.edges)
    {
        verify_out << u << ' ' << v << '\n';
    }
}
