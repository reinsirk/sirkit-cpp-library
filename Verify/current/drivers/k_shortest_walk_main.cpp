// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

int main()
{
    int n, m, source, target;
    std::size_t k = 0;
    verify_in >> n >> m >> source >> target >> k;
    std::vector<KShortestWalkEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
    }

    const auto lengths = k_shortest_walk_lengths(n, source, target, edges, k);
    for (const auto length : lengths)
    {
        verify_out << length << '\n';
    }
    for (auto i = lengths.size(); i < k; ++i)
    {
        verify_out << -1 << '\n';
    }
}
