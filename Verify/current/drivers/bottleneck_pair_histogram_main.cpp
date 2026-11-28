// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<sirkit::BottleneckEdge> edges(n - 1);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v >> weight;
        --u;
        --v;
    }
    const auto counts = sirkit::bottleneck_pair_histogram(n, edges);
    std::uint64_t total = 0;
    for (const auto &entry : counts)
    {
        total += static_cast<std::uint64_t>(entry.weight) * entry.minimax_pairs;
    }
    verify_out << total << '\n';
}
