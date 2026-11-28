// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int tests{};
    verify_in >> tests;
    while (tests-- != 0)
    {
        int n{};
        verify_in >> n;
        std::vector<std::uint64_t> weights(n);
        for (auto &weight : weights)
        {
            verify_in >> weight;
        }
        std::vector<std::pair<int, int>> edges(n - 1);
        for (auto &[u, v] : edges)
        {
            verify_in >> u >> v;
            --u;
            --v;
        }
        const auto extra = sirkit::augment_subcubic_tree_biconnectivity(weights, edges);
        verify_out << extra.edges.size() << '\n';
        for (const auto &[u, v] : extra.edges)
        {
            verify_out << u + 1 << ' ' << v + 1 << '\n';
        }
    }
}
