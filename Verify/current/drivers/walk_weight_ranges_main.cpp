// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstdlib>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::WalkWeightEdge> edges;
    edges.reserve(2 * m);
    for (int i = 0; i < m; ++i)
    {
        int u{}, v{}, bit{};
        verify_in >> u >> v >> bit;
        --u;
        --v;
        const int weight = 1 - 2 * bit;
        // Equal adjacent bits cancel: the alternating signed sum is unchanged.
        edges.push_back({2 * u, 2 * v + 1, weight});
        edges.push_back({2 * u + 1, 2 * v, -weight});
    }
    const auto ranges = sirkit::walk_weight_ranges(2 * n, edges, 0);
    for (int vertex = 0; vertex < n; ++vertex)
    {
        const auto even = ranges[2 * vertex];
        const auto odd = ranges[2 * vertex + 1];
        if (even.unbounded || odd.unbounded)
        {
            verify_out << -1 << '\n';
        }
        else
        {
            verify_out << std::max({std::abs(even.minimum), std::abs(even.maximum),
                                    std::abs(odd.minimum), std::abs(odd.maximum)})
                       << '\n';
        }
    }
}
