// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    int n, m;
    verify_in >> n >> m;
    IncrementalMinimumSpanningForest forest(n);
    for (int i = 0; i < m; ++i)
    {
        int u, v;
        std::int64_t weight = 0;
        verify_in >> u >> v >> weight;
        const auto update = forest.add_edge(u, v, weight);
        if (i != 0)
        {
            verify_out << ' ';
        }
        if (!update.selected)
        {
            verify_out << update.edge_id;
        }
        else if (update.removed_edge)
        {
            verify_out << *update.removed_edge;
        }
        else
        {
            verify_out << -1;
        }
    }
    verify_out << '\n';
}
