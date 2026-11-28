// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Sparse_Knapsack.cpp"
#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::SparseKnapsack::Item> items{{3, 4}, {5, 9}, {4, 7}};
    const sirkit::SparseKnapsack knapsack(8, items);
    assert(knapsack.max_value() == 13);
    assert(knapsack.max_value(4) == 7);
    assert(knapsack.frontier().back().weight == 8);
}
