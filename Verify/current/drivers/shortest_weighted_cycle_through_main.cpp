// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::WeightedCycleEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to;
        --from;
        --to;
        weight = 1;
    }
    const auto optimum = sirkit::shortest_weighted_cycle(n, edges, true, 0);
    if (optimum)
    {
        verify_out << optimum->weight << '\n';
    }
    else
    {
        verify_out << "-1\n";
    }
}
