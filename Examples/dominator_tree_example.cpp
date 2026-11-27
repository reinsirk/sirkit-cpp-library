// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Dominator_Tree.cpp"
#include <cassert>
#include <vector>

int main()
{
    const std::vector<std::vector<int>> graph{{1, 2}, {3}, {3}, {4}, {}, {}};
    const auto parent = sirkit::dominator_tree(graph, 0);
    assert((parent == std::vector<int>{0, 0, 0, 0, 3, -1}));
}
