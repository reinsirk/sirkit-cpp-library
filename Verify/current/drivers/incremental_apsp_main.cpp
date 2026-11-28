// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    struct Query
    {
        int type{}, first{}, second{};
    };
    int n{}, m{}, q{};
    verify_in >> n >> m >> q;
    std::vector<sirkit::IncrementalAPSPEdge> edges(m);
    for (auto &[u, v, w] : edges)
    {
        verify_in >> u >> v >> w;
        --u;
        --v;
    }
    std::vector<Query> queries(q);
    std::vector<bool> closed(m);
    for (auto &[type, first, second] : queries)
    {
        verify_in >> type >> first;
        --first;
        if (type == 1)
        {
            closed[first] = true;
        }
        else
        {
            verify_in >> second;
            --second;
        }
    }
    std::vector<sirkit::IncrementalAPSPEdge> remaining;
    remaining.reserve(m);
    for (int id = 0; id < m; ++id)
    {
        if (!closed[id])
        {
            remaining.push_back(edges[id]);
        }
    }
    sirkit::IncrementalAPSP distances(n, remaining, true);
    std::vector<std::int64_t> answers;
    answers.reserve(q);
    for (auto it = queries.rbegin(); it != queries.rend(); ++it)
    {
        const auto [type, first, second] = *it;
        if (type == 1)
        {
            const auto edge = edges[first];
            distances.add_edge(edge.from, edge.to, edge.weight);
        }
        else
        {
            answers.push_back(distances.distance(first, second).value_or(-1));
        }
    }
    for (auto it = answers.rbegin(); it != answers.rend(); ++it)
    {
        verify_out << *it << '\n';
    }
}
