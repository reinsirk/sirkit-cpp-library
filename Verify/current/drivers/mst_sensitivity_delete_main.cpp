// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::SpanningTreeEdge> edges(m);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v >> weight;
        --u;
        --v;
    }
    const auto result = sirkit::minimum_spanning_forest_sensitivity(n, edges);
    std::int64_t total = 0;
    for (const auto id : result.forest)
    {
        total += edges[id].weight;
    }
    auto worst = total;
    for (const auto id : result.forest)
    {
        const auto replacement = result.exchange[id].value();
        worst = std::max(worst, total - edges[id].weight + edges[replacement].weight);
    }
    verify_out << worst << '\n';
}
