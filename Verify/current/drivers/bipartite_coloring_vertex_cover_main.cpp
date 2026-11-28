// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <array>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const auto colors = sirkit::bipartite_coloring(n, edges);
    std::array<int, 2> count{};
    std::vector<int> offset(n);
    for (int v = 0; v < n; ++v)
    {
        offset[v] = count[colors.color[v]]++;
    }
    sirkit::BipartiteMatching matching(count[0], count[1]);
    for (auto [u, v] : edges)
    {
        if (colors.color[u] == 1)
        {
            std::swap(u, v);
        }
        matching.add_edge(offset[u], offset[v]);
    }
    matching.solve();
    verify_out << matching.min_vertex_cover().size() << '\n';
}
