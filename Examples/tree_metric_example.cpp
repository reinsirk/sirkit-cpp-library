// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Metric.cpp"

#include <iostream>

int main()
{
    constexpr int distances[3][3]{{0, 2, 5}, {2, 0, 3}, {5, 3, 0}};
    const auto tree = sirkit::reconstruct_tree_metric(3, [&](int u, int v) { return distances[u][v]; });
    for (const auto &[u, v, weight] : tree.value())
    {
        std::cout << u << ' ' << v << ' ' << weight << '\n'; // 0 1 2, then 1 2 3
    }
}
