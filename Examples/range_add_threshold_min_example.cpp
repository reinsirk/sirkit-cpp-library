// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Range_Add_Threshold_Min.cpp"

#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    const std::array keys{2, 4, 3};
    const std::array<std::int64_t, 3> values{10, 20, 5};
    sirkit::RangeAddThresholdMin<int> range(keys, values);
    assert(range.minimum_at_least(3) == 5);
    range.add(1, 3, 7);
    assert(range.minimum_at_least(3) == 12);
    assert(range.minimum_at_least(0, 2, 3) == 27);
    assert(!range.minimum_at_least(5));
}
