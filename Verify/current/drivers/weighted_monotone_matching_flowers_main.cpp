// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    std::size_t n = 0;
    verify_in >> n;
    std::vector<sirkit::WeightedMatchEdge> edges(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        edges[i].left = i;
        verify_in >> edges[i].right;
    }
    for (auto &edge : edges)
    {
        verify_in >> edge.weight;
    }
    verify_out << sirkit::maximum_weight_monotone_matching(edges).weight << '\n';
}
