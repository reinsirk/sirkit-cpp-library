// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Cyclic_Inversion_Statistics.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::array values{2, 1, 3};
    sirkit::CyclicInversionStatistics counts(values);
    const std::vector<std::int64_t> initial{1, 1, 3};
    assert(counts.left_rotation_counts() == initial);
    assert(counts.inversion_count() == 1 && counts.rotation_inversion_sum() == 5);
    counts.swap_adjacent(0);
    assert(counts.inversion_count() == 0 && counts.rotation_inversion_sum() == 4);
}
