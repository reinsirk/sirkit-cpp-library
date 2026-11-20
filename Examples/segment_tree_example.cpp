// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Segment_Tree.cpp"
#include <cstdint>
#include <iostream>
#include <vector>

using namespace sirkit;
std::int64_t op(std::int64_t a, std::int64_t b) { return a + b; }
std::int64_t e() { return 0; }
bool at_most_six(std::int64_t value) { return value <= 6; }
int main() {
    segtree<std::int64_t, op, e> tree(std::vector<std::int64_t>{2, 0, 3, 5});
    tree.set(1, 4);
    tree.transform(3, [](std::int64_t x) { return x - 4; });
    std::cout << tree.prod(1, 4) << '\n'; // 8
    std::cout << tree.all_prod() << '\n'; // 10
    std::cout << tree.max_right<at_most_six>(0) << '\n'; // 2
    std::cout << tree.min_left(4, [](auto x) { return x <= 4; }) << '\n'; // 2
}
