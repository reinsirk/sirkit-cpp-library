// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Weighted_L1_Statistics.cpp"

#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    const std::array<std::int64_t, 3> coordinates{2, 5, 9};
    sirkit::WeightedL1Statistics values(coordinates);
    values.insert(2, 3);
    values.insert(9, 2);
    assert(values.minimizer_interval()->first == 2);
    assert(values.absolute_deviation(2) == 14);
    assert(values.erase(2, 2));
    assert(values.minimizer_interval()->first == 9);
}
