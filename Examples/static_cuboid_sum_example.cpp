// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Static_Cuboid_Sum.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array points{sirkit::StaticCuboidSumPoint{1, 2, 3, 8}, sirkit::StaticCuboidSumPoint{2, 2, 3, 4}};
    const std::array queries{sirkit::StaticCuboidSumQuery{{0, 0, 0}, {2, 3, 4}}};
    const auto answer = sirkit::static_cuboid_sum(points, queries);
    assert(answer[0] == 8);
}
