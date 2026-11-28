// SPDX-License-Identifier: CC0-1.0
#include "../Graph/DSU_On_Tree.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array parents{-1, 0, 0, 1};
    const std::array colors{0, 0, 1, 1};
    std::array<int, 2> frequency{};
    std::array<int, 4> distinct{};
    const sirkit::DsuOnTree tree(parents);
    tree.run(
        [&](int v) { ++frequency[colors[v]]; },
        [&](int v) { --frequency[colors[v]]; },
        [&](int v) { distinct[v] = (frequency[0] != 0) + (frequency[1] != 0); });
    assert((distinct == std::array{2, 2, 1, 1}));
}
