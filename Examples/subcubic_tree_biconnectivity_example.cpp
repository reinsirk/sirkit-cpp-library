// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Subcubic_Tree_Biconnectivity.cpp"

#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::uint64_t> weights{2, 3, 5};
    const std::vector<std::pair<int, int>> edges{{0, 1}, {1, 2}};
    const auto added = sirkit::augment_subcubic_tree_biconnectivity(weights, edges);
    std::cout << added.cost.value() << '\n';
    for (const auto &[u, v] : added.edges)
    {
        std::cout << u << ' ' << v << '\n';
    }
}
