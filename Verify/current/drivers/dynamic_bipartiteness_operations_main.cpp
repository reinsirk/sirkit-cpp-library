// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>

int main()
{
    int n;
    std::size_t operation_count;
    verify_in >> n >> operation_count;
    DynamicBipartiteness graph(n);
    for (std::size_t i = 0; i < operation_count; ++i)
    {
        int type;
        verify_in >> type;
        if (type == 0)
        {
            int u, v;
            verify_in >> u >> v;
            graph.add_edge(u, v);
        }
        else if (type == 1)
        {
            std::size_t edge_id;
            verify_in >> edge_id;
            graph.remove_edge(edge_id);
        }
        else
        {
            graph.query_bipartite();
        }
    }
    for (const bool bipartite : graph.solve())
    {
        verify_out << (bipartite ? "Yes\n" : "No\n");
    }
}
