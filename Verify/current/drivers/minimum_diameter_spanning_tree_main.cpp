// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

int main()
{
    int n, m;
    verify_in >> n >> m;
    std::vector<MinimumDiameterTreeEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
    }
    const auto result = minimum_diameter_spanning_tree(n, edges);
    verify_out << result->diameter << '\n';
    for (std::size_t i = 0; i < result->edge_ids.size(); ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << result->edge_ids[i];
    }
    verify_out << '\n';
}
