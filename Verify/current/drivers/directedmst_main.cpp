// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n, m, root;
    verify_in >> n >> m >> root;
    std::vector<DirectedMstEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
    }

    const auto result = minimum_directed_spanning_tree(n, root, edges);
    verify_out << result->cost << '\n';
    for (int v = 0; v < n; ++v)
    {
        if (v != 0)
        {
            verify_out << ' ';
        }
        verify_out << (v == root ? root : edges[result->incoming_edge[v]].from);
    }
    verify_out << '\n';
}
