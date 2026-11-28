// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <numeric>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{}, k{};
    verify_in >> n >> m >> k;
    std::vector<int> visits(m);
    for (int &v : visits)
    {
        verify_in >> v;
        --v;
    }
    std::vector<std::pair<int, int>> edges(n - 1);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    sirkit::TreePathDifference<int> paths(n, edges);
    for (int i = 1; i < m; ++i)
    {
        paths.add_edge_path(visits[i - 1], visits[i], 1);
    }
    const auto values = paths.edge_values();
    const int total = std::accumulate(values.begin(), values.end(), 0);
    const int doubled = total + k;
    if (doubled < 0 || doubled > 2 * total || doubled % 2)
    {
        verify_out << 0 << '\n';
        return 0;
    }
    const int target = doubled / 2;
    constexpr int kMod = 998244353;
    std::vector<int> ways(target + 1);
    ways[0] = 1;
    for (const int cost : values)
    {
        for (int value = target; value >= cost; --value)
        {
            ways[value] += ways[value - cost];
            if (ways[value] >= kMod)
            {
                ways[value] -= kMod;
            }
        }
    }
    verify_out << ways[target] << '\n';
}
