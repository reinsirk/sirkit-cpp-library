// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    std::uint64_t capacity{};
    verify_in >> n >> capacity;
    std::vector<sirkit::SparseKnapsack::Item> items(n);
    for (auto &[weight, value] : items)
    {
        verify_in >> value >> weight;
    }
    const sirkit::SparseKnapsack knapsack(capacity, items);
    verify_out << knapsack.max_value() << '\n';
}
