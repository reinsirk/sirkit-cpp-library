// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Steiner_Tree.cpp"
#include <array>
#include <cassert>

int main()
{
    const std::array<sirkit::SteinerTreeEdge, 3> edges{{{0, 1, 1}, {1, 2, 2}, {0, 2, 7}}};
    const std::array terminals{0, 2};
    const auto tree = sirkit::minimum_steiner_tree(3, edges, terminals);
    assert(tree && tree->cost == 3 && tree->edge_ids.size() == 2);
}
