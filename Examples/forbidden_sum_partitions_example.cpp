// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Forbidden_Sum_Partitions.cpp"

#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    const std::array<std::int64_t, 3> values{1, 2, 3};
    const auto count = sirkit::count_partitions_avoiding_sums<std::uint64_t>(values, 3);
    assert(count == 2);
}
