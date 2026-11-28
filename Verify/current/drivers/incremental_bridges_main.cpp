// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{}, source{}, target{};
    verify_in >> n >> m >> source >> target;
    --source;
    --target;
    sirkit::IncrementalBridges bridges(n);
    bridges.reserve_edges(static_cast<std::size_t>(m));
    std::vector<std::vector<std::pair<int, std::size_t>>> graph(n);
    for (int i = 0; i < m; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        --u;
        --v;
        const auto id = bridges.add_edge(u, v);
        graph[u].emplace_back(v, id);
        graph[v].emplace_back(u, id);
    }
    std::vector<int> distance(n, -1), cycle_edges(n, -1), queue{source};
    distance[source] = 0;
    cycle_edges[source] = 0;
    for (std::size_t i = 0; i < queue.size(); ++i)
    {
        const int u = queue[i];
        for (const auto &[v, id] : graph[u])
        {
            if (distance[v] == -1)
            {
                distance[v] = distance[u] + 1;
                queue.push_back(v);
            }
            if (distance[v] == distance[u] + 1)
            {
                cycle_edges[v] = std::max(cycle_edges[v], cycle_edges[u] + !bridges.is_bridge(id));
            }
        }
    }
    verify_out << cycle_edges[target] << '\n';
}
