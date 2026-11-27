// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Furthest_Pair.cpp"
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::pair<std::int32_t, std::int32_t>> points{{0, 0}, {3, 4}, {1, 1}};
    const auto result = sirkit::furthest_pair(points);
    assert(result && result->distance_squared == 25);
    assert(result->first_index == 0 && result->second_index == 1);
}
