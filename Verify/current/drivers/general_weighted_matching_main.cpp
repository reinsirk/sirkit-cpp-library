// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::GeneralWeightedMatchingEdge> edges(m);
    for (auto &edge : edges)
    {
        verify_in >> edge.u >> edge.v >> edge.weight;
    }
    const auto answer = sirkit::maximum_weight_matching(n, edges);
    verify_out << answer.edges.size() << ' ' << answer.weight << '\n';
    for (const auto &[u, v] : answer.edges)
    {
        verify_out << u << ' ' << v << '\n';
    }
}
