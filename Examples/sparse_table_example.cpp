// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Sparse_Table.cpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>
struct Minimum { int operator()(int a, int b) const { return std::min(a, b); } };
int main() {
    SparseTable<int, Minimum> table(std::vector<int>{4, 2, 7, 1, 8}, std::numeric_limits<int>::max());
    std::cout << table.prod(1, 4) << '\n'; // 1
    std::cout << table.get(2) << '\n'; // 7
    std::cout << table.max_right(0, [](int x) { return x >= 2; }) << '\n'; // 3
    std::cout << table.min_left(5, [](int x) { return x >= 2; }) << '\n'; // 4
}
