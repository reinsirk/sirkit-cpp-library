// SPDX-License-Identifier: CC0-1.0
#include "Graph/Hub_Sparse_Distances.cpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Shortcut = sirkit::HubSparseDistances::Shortcut;

bool agrees(const std::vector<std::uint64_t> &gates, const std::vector<Shortcut> &edges)
{
    const auto n = gates.size();
    std::vector<std::vector<std::uint64_t>> graph(n, std::vector<std::uint64_t>(n));
    for (std::size_t u = 0; u < n; ++u)
    {
        for (std::size_t v = u + 1; v < n; ++v)
        {
            graph[u][v] = graph[v][u] = gates[u] + gates[v];
        }
    }
    for (const auto &edge : edges)
    {
        auto &d = graph[edge.from][edge.to];
        d = std::min(d, edge.cost);
        graph[edge.to][edge.from] = d;
    }
    const sirkit::HubSparseDistances compressed(gates, edges);
    if (compressed.size() != n)
    {
        return false;
    }
    for (std::size_t source = 0; source < n; ++source)
    {
        constexpr auto kInfinity = std::numeric_limits<std::uint64_t>::max();
        std::vector<std::uint64_t> d(n, kInfinity);
        std::vector<char> visited(n);
        d[source] = 0;
        for (std::size_t step = 0; step < n; ++step)
        {
            std::size_t best = n;
            for (std::size_t v = 0; v < n; ++v)
            {
                if (visited[v] == 0 && (best == n || d[v] < d[best]))
                {
                    best = v;
                }
            }
            visited[best] = 1;
            for (std::size_t v = 0; v < n; ++v)
            {
                if (d[best] <= kInfinity - graph[best][v])
                {
                    d[v] = std::min(d[v], d[best] + graph[best][v]);
                }
            }
        }
        for (std::size_t v = 0; v < n; ++v)
        {
            if (d[v] != compressed.distance(static_cast<int>(source), static_cast<int>(v)))
            {
                return false;
            }
        }
    }
    return true;
}
}

int main()
{
    constexpr auto max_gate = std::numeric_limits<std::uint64_t>::max() / 2;
    if (!agrees({}, {}) || !agrees({max_gate, max_gate, max_gate, 5}, {{0, 1, max_gate}, {1, 2, max_gate}, {0, 0, 0}, {0, 2, max_gate - 1}}) ||
        !agrees({10, 100, 100, 5}, {{0, 1, 1}, {1, 2, 1}, {0, 2, 1000}, {0, 1, 0}}))
    {
        return 1;
    }
    std::mt19937_64 random(745562);
    for (int n = 1; n <= 20; ++n)
    {
        for (int iteration = 0; iteration < 24; ++iteration)
        {
            std::vector<std::uint64_t> gates(n);
            for (auto &gate : gates)
            {
                gate = iteration % 2 == 0 ? random() % 40 : max_gate - random() % 200;
            }
            const auto edge_count = random() % (3 * n);
            std::vector<Shortcut> edges;
            for (std::uint64_t i = 0; i < edge_count; ++i)
            {
                const auto u = static_cast<int>(random() % n);
                const auto v = static_cast<int>(random() % n);
                const auto cost = iteration % 2 == 0 ? random() % 45 : random();
                edges.push_back({u, v, i % 8 == 0 ? 0 : cost});
            }
            if (!agrees(gates, edges))
            {
                return 2;
            }
        }
    }
    const std::vector<std::uint64_t> gates{11, 8};
    const sirkit::HubSparseDistances graph(gates, {});
    try
    {
        static_cast<void>(graph.distance(0, -1));
        return 3;
    }
    catch (const std::out_of_range &)
    {
    }
    try
    {
        const std::vector<Shortcut> invalid{{2, 0, 4}};
        static_cast<void>(sirkit::HubSparseDistances(gates, invalid));
        return 4;
    }
    catch (const std::out_of_range &)
    {
    }
    try
    {
        const std::vector<std::uint64_t> oversized{max_gate + 1};
        static_cast<void>(sirkit::HubSparseDistances(oversized, {}));
        return 5;
    }
    catch (const std::invalid_argument &)
    {
    }
}
