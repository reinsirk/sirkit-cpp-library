// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <numeric>
#include <vector>

struct EdgeInput
{
    int from, to;
    std::uint32_t weight;
};

int main()
{
    int n, m, start, goal;
    verify_in >> n >> m >> start >> goal;
    std::vector<int> offset(static_cast<std::size_t>(n) + 1);
    std::vector<EdgeInput> input(static_cast<std::size_t>(m));
    for (auto &edge : input)
    {
        verify_in >> edge.from >> edge.to >> edge.weight;
        ++offset[static_cast<std::size_t>(edge.from) + 1];
    }
    std::partial_sum(offset.begin(), offset.end(), offset.begin());
    auto cursor = offset;
    std::vector<int> to(static_cast<std::size_t>(m));
    std::vector<std::uint32_t> weight(static_cast<std::size_t>(m));
    for (const auto &edge : input)
    {
        const auto index = static_cast<std::size_t>(cursor[static_cast<std::size_t>(edge.from)]++);
        to[index] = edge.to;
        weight[index] = edge.weight;
    }

    constexpr auto kUnreachable = std::numeric_limits<std::uint64_t>::max();
    std::vector<std::uint64_t> distance(static_cast<std::size_t>(n), kUnreachable);
    std::vector<int> parent(static_cast<std::size_t>(n), -1);
    sirkit::RadixHeap<std::uint64_t, int> queue;
    queue.emplace(0, start);
    distance[static_cast<std::size_t>(start)] = 0;
    while (!queue.empty())
    {
        const auto [current, vertex] = queue.pop();
        const auto index = static_cast<std::size_t>(vertex);
        if (vertex == goal)
        {
            break;
        }
        if (current != distance[index])
        {
            continue;
        }
        for (int edge = offset[index]; edge < offset[index + 1]; ++edge)
        {
            const auto edge_index = static_cast<std::size_t>(edge);
            const auto next = static_cast<std::size_t>(to[edge_index]);
            const auto candidate = current + weight[edge_index];
            if (candidate < distance[next])
            {
                distance[next] = candidate;
                parent[next] = vertex;
                queue.emplace(candidate, to[edge_index]);
            }
        }
    }

    if (distance[static_cast<std::size_t>(goal)] == kUnreachable)
    {
        verify_out << -1 << '\n';
        return 0;
    }
    std::vector<int> path;
    for (int vertex = goal; vertex != start; vertex = parent[static_cast<std::size_t>(vertex)])
    {
        path.push_back(vertex);
    }
    verify_out << distance[static_cast<std::size_t>(goal)] << ' ' << path.size() << '\n';
    int previous = start;
    for (auto it = path.rbegin(); it != path.rend(); ++it)
    {
        verify_out << previous << ' ' << *it << '\n';
        previous = *it;
    }
}
