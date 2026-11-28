// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
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
    std::vector<bool> selected(m);
    for (const auto id : result.forest)
    {
        total += edges[id].weight;
        selected[id] = true;
    }
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        if (selected[i])
        {
            verify_out << total << '\n';
        }
        else
        {
            const auto remove = result.exchange[i].value();
            verify_out << total - edges[remove].weight + edges[i].weight << '\n';
        }
    }
}
