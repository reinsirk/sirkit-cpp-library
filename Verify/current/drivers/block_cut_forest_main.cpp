// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <utility>
#include <vector>

std::vector<int> distances(const sirkit::Graph &graph, int start)
{
    std::vector<int> distance(graph.size(), -1), queue{start};
    queue.reserve(graph.size());
    distance[start] = 0;
    for (std::size_t i = 0; i < queue.size(); ++i)
    {
        const int v = queue[i];
        for (const int u : graph[v])
        {
            if (distance[u] == -1)
            {
                distance[u] = distance[v] + 1;
                queue.push_back(u);
            }
        }
    }
    return distance;
}

int main()
{
    int n{}, m{}, a{}, b{}, c{};
    verify_in >> n >> m >> a >> b >> c;
    --a;
    --b;
    --c;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const auto forest = sirkit::block_cut_forest(n, edges);
    const auto from_a = distances(forest, a);
    const auto from_b = distances(forest, b);
    // BからA-Cの木上経路までの距離が1以下なら、経路上の頂点かブロックを共有する。
    const bool present = from_b[a] + from_b[c] - from_a[c] <= 2;
    verify_out << (present ? "Yes\n" : "No\n");
}
