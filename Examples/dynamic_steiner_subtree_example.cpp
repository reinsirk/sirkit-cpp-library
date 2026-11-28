// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Dynamic_Steiner_Subtree.cpp"

#include <array>
#include <iostream>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
    sirkit::DynamicSteinerSubtree tree(3, edges);
    tree.set_marked(0);
    tree.set_marked(2);
    std::cout << tree.steiner_vertices() << '\n';
    std::cout << tree.steiner_vertices(1, 2) << '\n';
}
