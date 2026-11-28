// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::XorWalkEdge> edges(m);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v >> weight;
        --u;
        --v;
    }
    const sirkit::XorWalkReachability walks(n, edges, 0);
    if (const auto answer = walks.min_xor(n - 1))
    {
        verify_out << *answer << '\n';
    }
    else
    {
        verify_out << -1 << '\n';
    }
}
