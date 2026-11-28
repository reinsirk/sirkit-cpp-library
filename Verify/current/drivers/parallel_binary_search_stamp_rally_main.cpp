// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    std::size_t vertex_count = 0, edge_count = 0;
    verify_in >> vertex_count >> edge_count;
    std::vector<std::pair<int, int>> edges(edge_count);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    struct Query
    {
        int first, second, target;
    };
    std::size_t query_count = 0;
    verify_in >> query_count;
    std::vector<Query> queries(query_count);
    for (auto &[first, second, target] : queries)
    {
        verify_in >> first >> second >> target;
        --first;
        --second;
    }
    sirkit::UnionFind forest(vertex_count);
    const auto answers = sirkit::parallel_binary_search(
        edges.size(), queries.size(),
        [&] { forest.reset(); },
        [&](std::size_t i) { forest.unite(edges[i].first, edges[i].second); },
        [&](std::size_t i) {
            const auto [first, second, target] = queries[i];
            auto count = forest.size(first);
            if (!forest.same(first, second))
            {
                count += forest.size(second);
            }
            return count >= target;
        });
    for (const auto answer : answers)
    {
        verify_out << answer << '\n';
    }
}
