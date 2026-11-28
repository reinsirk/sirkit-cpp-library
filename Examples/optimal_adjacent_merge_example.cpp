// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Optimal_Adjacent_Merge.cpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<std::uint64_t> weights{10, 20, 30, 40};
    const auto result = sirkit::optimal_adjacent_merge(weights);
    assert(result.cost == 190);
    const std::vector<std::array<std::size_t, 2>> expected{{0, 1}, {4, 2}, {5, 3}};
    assert(result.merges == expected);
}
