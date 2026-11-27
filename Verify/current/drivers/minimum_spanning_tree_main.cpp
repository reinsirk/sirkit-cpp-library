// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<SpanningTreeEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
    }
    const auto tree = minimum_spanning_forest(n, edges);
    std::int64_t total = 0;
    for (std::size_t id : tree)
    {
        total += edges[id].weight;
    }
    verify_out << total << '\n';
    for (std::size_t i = 0; i < tree.size(); ++i)
    {
        verify_out << tree[i] << (i + 1 == tree.size() ? '\n' : ' ');
    }
}
