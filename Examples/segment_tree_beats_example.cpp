// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Segment_Tree_Beats.cpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
int main() {
    SegmentTreeBeats tree(std::vector<std::int64_t>{1, 5, -2, 8});
    tree.chmin(0, 4, 4);
    tree.chmax(1, 3, 0);
    tree.add(0, 2, 3);
    tree.set(2, -1);
    assert(tree.get(2) == -1 && tree.sum(0, 4) == 14);
    assert(tree.min(0, 4) == -1 && tree.max(0, 4) == 7);
    assert(!tree.min(2, 2) && !tree.max(2, 2) && tree.sum(2, 2) == 0);
    for (auto value : tree.values()) std::cout << value << ' ';
    std::cout << '\n';
    auto copy = tree;
    copy.add(0, copy.size(), 1);
    tree.swap(copy);
    tree.reset({10, -3});
    assert(tree.size() == 2 && tree.sum(0, 2) == 7);
    tree.clear();
    assert(tree.empty());
}
