// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Offline_Rectangle_Add_Point_Get.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<sirkit::RectangleAdd> initial{{0, 0, 3, 3, 5}};
    const std::vector<sirkit::RectangleAddPointGetOperation> operations{
        sirkit::RectanglePointGet{1, 1},
        sirkit::RectangleAdd{1, 1, 2, 2, -3},
        sirkit::RectanglePointGet{1, 1},
        sirkit::RectanglePointGet{2, 1},
    };
    const auto answers = sirkit::offline_rectangle_add_point_get(initial, operations);
    assert((answers == std::vector<std::int64_t>{5, 2, 5}));
}
