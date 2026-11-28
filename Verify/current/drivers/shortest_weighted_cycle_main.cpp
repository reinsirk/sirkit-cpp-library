// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int type{}, n{}, m{};
    verify_in >> type >> n >> m;
    std::vector<sirkit::WeightedCycleEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
        --from;
        --to;
    }
    const auto optimum = sirkit::shortest_weighted_cycle(n, edges, type == 1);
    if (optimum)
    {
        verify_out << optimum->weight << '\n';
    }
    else
    {
        verify_out << "-1\n";
    }
}
