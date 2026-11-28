// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Knapsack_By_Value.cpp"
#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::KnapsackByValue::Item> items{{3, 4}, {5, 9}, {4, 7}};
    const sirkit::KnapsackByValue knapsack(items);
    assert(knapsack.max_value(8) == 13);
    assert(knapsack.minimum_weight(11) == 7);
    assert(!knapsack.minimum_weight(3));
}
