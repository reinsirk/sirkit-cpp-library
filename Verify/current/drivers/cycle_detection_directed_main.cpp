// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n, m;
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
    }
    const auto cycle = find_cycle(n, edges, true);
    if (!cycle)
    {
        verify_out << -1 << '\n';
        return 0;
    }
    verify_out << cycle->vertices.size() << '\n';
    for (auto id : cycle->edge_ids)
    {
        verify_out << id << '\n';
    }
}
