// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Range_Fenwick_Tree.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<std::int64_t> initial{2, 1, 4, 7};
    sirkit::RangeFenwickTree<> tree(initial);
    tree.add(1, 4, 3);
    assert(tree.sum(0, 3) == 13);
    assert(tree.get(3) == 10);
    tree.set(0, 5);
    assert(tree.sum(0, 4) == 26);
}
