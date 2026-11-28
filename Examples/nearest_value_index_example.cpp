// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Nearest_Value_Index.cpp"

#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    const std::array<std::int64_t, 4> values{-3, -1, 5, 6};
    const sirkit::NearestValueIndex points(values);
    const auto third = points.kth_nearest(-2, 3);
    assert(third && third->value == 5 && third->distance == 7);
    assert(points.count_within(-2, 1) == 2);
}
