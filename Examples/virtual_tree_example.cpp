// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Virtual_Tree.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{1, 3}, std::pair{3, 4}};
    const sirkit::VirtualTree tree(5, edges);
    const std::array terminals{4, 2};
    const auto nodes = tree.build(terminals);
    assert(nodes.size() == 3);
    assert(nodes[0].vertex == 1 && nodes[0].parent == -1 && !nodes[0].terminal);
    assert(nodes[1].parent == 0 && nodes[2].parent == 0);
    assert(nodes[1].distance + nodes[2].distance == 3);
}
