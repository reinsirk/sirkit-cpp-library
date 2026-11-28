// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{}, source{}, target{};
    std::size_t k{};
    verify_in >> n >> m >> k >> source >> target;
    --source;
    --target;
    std::vector<std::pair<int, int>> points(n);
    for (auto &[x, y] : points)
    {
        verify_in >> x >> y;
    }
    constexpr double kScale = 1e9;
    std::vector<sirkit::ShortestPathEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to;
        --from;
        --to;
        const auto [x, y] = points[from];
        const auto [u, v] = points[to];
        weight = static_cast<std::uint64_t>(std::llround(std::hypot(x - u, y - v) * kScale));
    }
    const auto paths = sirkit::k_shortest_simple_paths(n, edges, source, target, k, true);
    verify_out.fixed(9);
    for (std::size_t i = 0; i < k; ++i)
    {
        if (i < paths.size())
        {
            verify_out << static_cast<double>(paths[i].distance) / kScale << '\n';
        }
        else
        {
            verify_out << -1 << '\n';
        }
    }
}
