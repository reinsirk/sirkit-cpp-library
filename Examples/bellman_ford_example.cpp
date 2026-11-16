// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Bellman-Ford.cpp"
#include <iostream>

using namespace sirkit;

int main()
{
    const std::int64_t inf = 1'000'000'000'000; // 省略時はkInf。
    WGraph graph(5);
    graph[0].emplace_back(1, 2);
    graph[1].emplace_back(2, -3);
    graph[2].emplace_back(1, 1); // 負閉路。
    graph[2].emplace_back(3, 4);
    std::vector<std::int64_t> distances;
    std::cout << BellmanFord(graph, distances, 0, inf) << '\n'; // 1
    for (const auto distance : distances)
    {
        if (distance == inf)
            std::cout << "unreachable\n";
        else if (distance == -inf)
            std::cout << "negative cycle\n";
        else
            std::cout << distance << '\n';
    }
    std::cout << BellmanFord(graph, distances, 4, inf) << ' ' << distances[4] << '\n'; // 0 0

}
