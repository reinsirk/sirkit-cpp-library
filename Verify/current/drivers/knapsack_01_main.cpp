// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

int main()
{
    std::size_t n, capacity;
    verify_in >> n >> capacity;
    std::vector<KnapsackItem> items(n);
    for (auto &item : items)
    {
        verify_in >> item.weight >> item.value;
    }
    verify_out << maximize_knapsack(capacity, items) << '\n';
}
