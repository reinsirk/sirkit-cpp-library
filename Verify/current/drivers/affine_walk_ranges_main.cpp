// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::AffineWalkEdge> edges(m);
    for (auto &edge : edges)
    {
        int bit{};
        verify_in >> edge.from >> edge.to >> bit;
        --edge.from;
        --edge.to;
        edge.reflect = true;
        edge.shift = 1 - 2 * bit;
    }
    for (const auto &range : sirkit::affine_walk_ranges(n, edges, 0))
    {
        verify_out << (range.unbounded ? -1 : std::max(-range.minimum, range.maximum)) << '\n';
    }
}
