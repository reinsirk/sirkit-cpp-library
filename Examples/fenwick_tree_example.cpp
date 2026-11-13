// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Fenwick_Tree.cpp"
#include <cstdint>
#include <iostream>
#include <vector>
int main() {
    fenwick_tree<std::int64_t> sums(std::vector<std::int64_t>{2, 0, 3, 5});
    sums.add(1, 4);
    sums.set(3, 1);
    std::cout << sums.sum(1, 4) << '\n'; // 8
    std::cout << sums.get(1) << '\n'; // 4
    std::cout << sums.lower_bound(7) << '\n'; // 2
    std::cout << sums.upper_bound(9) << '\n'; // 3
    for (auto x : sums.values()) std::cout << x << ' ';
    std::cout << '\n'; // 2 4 3 1
}
