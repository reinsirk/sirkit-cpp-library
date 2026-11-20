// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Disjoint_Sparse_Table.cpp"
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>

using namespace sirkit;
int main() {
    DisjointSparseTable<std::int64_t, std::plus<std::int64_t>> table(std::vector<std::int64_t>{2, 0, 3, 5});
    std::cout << table.prod(1, 4) << '\n'; // 8
    std::cout << table.all_prod() << '\n'; // 10
    std::cout << table.max_right(0, [](auto x) { return x <= 5; }) << '\n'; // 3
    std::cout << table.min_left(4, [](auto x) { return x <= 5; }) << '\n'; // 3
}
