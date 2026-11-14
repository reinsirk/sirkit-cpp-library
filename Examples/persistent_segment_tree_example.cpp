// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Persistent_Segment_Tree.cpp"
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>

using namespace sirkit;
int main() {
    PersistentSegmentTree<std::int64_t, std::plus<std::int64_t>> tree(
        std::vector<std::int64_t>{2, 4, 1}, {}, 0);
    tree.reserve(64);
    const auto first = tree.latest_root();
    const auto second = tree.set(first, 1, 9);
    const auto third = tree.transform(first, 2, [](auto x) { return x + 5; });
    std::cout << tree.all_prod(first) << '\n';  // 7
    std::cout << tree.all_prod(second) << '\n'; // 12
    std::cout << tree.prod(third, 1, 3) << '\n'; // 10
    std::cout << tree.get(second, 1) << '\n';   // 9
    std::cout << tree.max_right(first, 0, [](auto x) { return x <= 6; }) << '\n'; // 2
    std::cout << tree.min_left(first, 3, [](auto x) { return x <= 5; }) << '\n'; // 1
    auto copy = tree;
    auto copied = copy.remap_from(tree, first);
    std::cout << copy.all_prod(copy.set(copied, 0, 10)) << '\n'; // 15
    std::cout << tree.all_prod(tree.empty_root()) << '\n'; // 0
    auto rebuilt = tree.build(std::vector<std::int64_t>{1, 1, 1});
    std::cout << tree.all_prod(rebuilt) << ' ' << tree.size() << ' ' << tree.empty() << '\n'; // 3 3 0
    std::cout << (tree.allocated_nodes() > 0) << '\n'; // 1
    tree.reset(5);
    std::cout << tree.all_prod(tree.latest_root()) << '\n'; // 0
    tree.clear();
    std::cout << tree.size() << '\n'; // 0
}
