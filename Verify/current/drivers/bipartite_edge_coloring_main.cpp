// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int left, right, m;
    verify_in >> left >> right >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
    }

    const auto answer = bipartite_edge_coloring(left, right, edges);
    verify_out << answer.color_count << '\n';
    for (const auto color : answer.colors)
    {
        verify_out << color << '\n';
    }
}
