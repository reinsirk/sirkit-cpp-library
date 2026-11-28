// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<std::int64_t> coordinate(n), negative(n);
    for (int i = 0; i < n; ++i)
    {
        verify_in >> coordinate[i];
        negative[i] = -coordinate[i];
    }
    const auto graph = [&] {
        sirkit::RangeEdgeGraph network(n);
        const auto east = network.add_layer(negative, coordinate);
        const auto west = network.add_layer(coordinate, negative);
        for (int i = 0; i < m; ++i)
        {
            int board_left{}, board_right{}, arrive_left{}, arrive_right{};
            std::int64_t fare{};
            verify_in >> board_left >> board_right >> arrive_left >> arrive_right >> fare;
            const auto direction = board_right < arrive_left ? east : west;
            network.add_range_edge(board_left - 1, board_right, arrive_left - 1, arrive_right, fare, direction);
        }
        return network.graph();
    }();

    constexpr std::int64_t kInfinity = std::int64_t{1} << 60;
    std::vector<std::int64_t> distance(graph.size(), kInfinity);
    using State = std::pair<std::int64_t, int>;
    std::priority_queue<State, std::vector<State>, std::greater<>> queue;
    distance[0] = 0;
    queue.emplace(0, 0);
    while (!queue.empty())
    {
        const auto [current, v] = queue.top();
        queue.pop();
        if (current != distance[v])
        {
            continue;
        }
        for (const auto &edge : graph[v])
        {
            const auto next = current + edge.w;
            if (next < distance[edge.to])
            {
                distance[edge.to] = next;
                queue.emplace(next, edge.to);
            }
        }
    }
    for (int v = 1; v < n; ++v)
    {
        if (v > 1)
        {
            verify_out << ' ';
        }
        verify_out << (distance[v] == kInfinity ? -1 : distance[v]);
    }
    verify_out << '\n';
}
