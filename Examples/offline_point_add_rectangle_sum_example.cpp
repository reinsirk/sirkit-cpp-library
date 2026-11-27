// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Offline_Point_Add_Rectangle_Sum.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<sirkit::PointAdd> initial{{0, 0, 4}};
    const std::vector<sirkit::PointAddRectangleSumOperation> operations{
        sirkit::RectangleSumRequest{0, 0, 2, 2},
        sirkit::PointAdd{0, 0, -1},
        sirkit::RectangleSumRequest{0, 0, 2, 2},
    };
    const auto answers = sirkit::offline_point_add_rectangle_sum(initial, operations);
    assert((answers == std::vector<std::int64_t>{4, 3}));
}
