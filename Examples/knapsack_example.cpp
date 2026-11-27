// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Knapsack.cpp"
#include <cassert>

int main()
{
    const std::vector<sirkit::KnapsackItem> items{{2, 3, 3}, {5, 8}};
    assert(sirkit::maximize_knapsack(7, items) == 11);
    assert(!sirkit::maximize_exact_knapsack(3, items));
}
