// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

int main()
{
    std::size_t n, m, start, goal;
    verify_in >> n >> m >> start >> goal;
    std::vector<std::vector<std::pair<std::size_t, std::uint32_t>>> graph(n);
    for (std::size_t i = 0; i < m; ++i)
    {
        std::size_t from, to;
        std::uint32_t weight;
        verify_in >> from >> to >> weight;
        graph[from].emplace_back(to, weight);
    }
    constexpr auto kUnreachable = std::numeric_limits<std::uint64_t>::max();
    std::vector<std::uint64_t> distance(n, kUnreachable);
    std::vector<std::size_t> parent(n);
    sirkit::IndexedPriorityQueue<std::uint64_t> queue(n);
    queue.set(start, 0);
    distance[start] = 0;
    while (!queue.empty())
    {
        const auto [vertex, key] = queue.pop();
        if (vertex == goal)
        {
            break;
        }
        for (const auto &[to, weight] : graph[vertex])
        {
            const auto candidate = key + weight;
            if (candidate < distance[to])
            {
                distance[to] = candidate;
                parent[to] = vertex;
                queue.set(to, candidate);
            }
        }
    }
    if (distance[goal] == kUnreachable)
    {
        verify_out << -1 << '\n';
        return 0;
    }
    std::vector<std::size_t> path;
    for (std::size_t vertex = goal; vertex != start; vertex = parent[vertex])
    {
        path.push_back(vertex);
    }
    verify_out << distance[goal] << ' ' << path.size() << '\n';
    auto previous = start;
    for (auto it = path.rbegin(); it != path.rend(); ++it)
    {
        verify_out << previous << ' ' << *it << '\n';
        previous = *it;
    }
}
