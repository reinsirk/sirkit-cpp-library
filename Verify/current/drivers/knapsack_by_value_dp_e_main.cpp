// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    std::uint64_t budget{};
    verify_in >> n >> budget;
    std::vector<sirkit::KnapsackByValue::Item> items(n);
    for (auto &[weight, value] : items)
    {
        verify_in >> weight >> value;
    }
    const sirkit::KnapsackByValue knapsack(items);
    verify_out << knapsack.max_value(budget) << '\n';
}
