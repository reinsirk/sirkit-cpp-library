// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{}, q{};
    verify_in >> n >> m >> q;
    std::vector<std::uint64_t> star(n);
    for (auto &weight : star)
    {
        verify_in >> weight;
    }
    std::vector<sirkit::DynamicStarCutEdge> edges(m);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v >> weight;
    }
    sirkit::DynamicStarMinCut cut(n, edges, star);
    for (int i = 0; i < q; ++i)
    {
        int v{};
        std::uint64_t weight{};
        verify_in >> v >> weight;
        verify_out << cut.set_star_weight(v, weight) << '\n';
    }
}
