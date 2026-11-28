// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Range_Add_Weighted_Minimum.cpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<std::uint64_t> weights{2, 3, 7};
    sirkit::RangeAddWeightedMinimum<> data(weights);
    data.add(1, 2, 4);
    const auto result = data.all_minimum().value();
    assert(result.value == 0 && result.weight == 9);
}
