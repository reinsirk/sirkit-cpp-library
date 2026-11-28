// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::ShortestPathEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
        --from;
        --to;
    }
    const auto result = sirkit::shortest_path_edge_usage(n, edges, 0, n - 1, true);
    for (const auto use : result.edge_use)
    {
        verify_out << (use == sirkit::ShortestPathEdgeUse::all ? "Yes\n" : "No\n");
    }
}
