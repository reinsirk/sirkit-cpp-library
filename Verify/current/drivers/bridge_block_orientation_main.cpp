// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m), equal_edges;
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    std::vector<int> reach(n);
    for (int &value : reach)
    {
        verify_in >> value;
    }
    for (const auto &[u, v] : edges)
    {
        if (reach[u] == reach[v])
        {
            equal_edges.emplace_back(u, v);
        }
    }
    const auto oriented = sirkit::orient_bridge_blocks(n, equal_edges);
    std::size_t next = 0;
    for (const auto &[u, v] : edges)
    {
        const bool forward = reach[u] == reach[v] ? oriented[next++].first == u : reach[u] > reach[v];
        verify_out << (forward ? "->\n" : "<-\n");
    }
}
