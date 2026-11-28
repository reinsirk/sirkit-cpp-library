// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Graph_Metric_Realization.cpp"

#include <iostream>

int main()
{
    constexpr int distance[3][3]{{0, 2, 7}, {2, 0, 5}, {7, 5, 0}};
    const auto graph = sirkit::realize_graph_metric(3, [&](int u, int v) { return distance[u][v]; });
    std::cout << graph->edges.size() << ' ' << graph->total_weight.value() << '\n'; // 2 7
}
