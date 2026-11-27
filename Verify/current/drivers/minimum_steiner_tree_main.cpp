// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <vector>

int main()
{
    int n, m;
    verify_in >> n >> m;
    std::vector<SteinerTreeEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
    }
    int k;
    verify_in >> k;
    std::vector<int> terminals(k);
    for (auto &v : terminals)
    {
        verify_in >> v;
    }
    const auto result = minimum_steiner_tree(n, edges, terminals);
    if (!result)
    {
        return 1;
    }
    verify_out << result->cost << ' ' << result->edge_ids.size() << '\n';
    for (std::size_t i = 0; i < result->edge_ids.size(); ++i)
    {
        verify_out << result->edge_ids[i] << (i + 1 == result->edge_ids.size() ? '\n' : ' ');
    }
}
