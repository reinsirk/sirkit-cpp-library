// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    int n;
    std::size_t m;
    verify_in >> n >> m;
    std::vector<GlobalMinCutEdge> edges(m);
    for (auto &[u, v, w] : edges)
    {
        verify_in >> u >> v >> w;
    }
    if (const auto cut = global_min_cut(n, edges))
    {
        verify_out << cut->weight << '\n';
    }
    else
    {
        verify_out << "-1\n";
    }
}
