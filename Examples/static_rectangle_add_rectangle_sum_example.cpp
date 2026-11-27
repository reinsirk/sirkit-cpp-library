// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Static_Rectangle_Add_Rectangle_Sum.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<sirkit::StaticRectangleAdd> additions{{0, 0, 3, 2, 5}};
    const std::vector<sirkit::StaticRectangleSumQuery> queries{{1, 0, 4, 2}, {3, 0, 4, 2}};
    const auto answers = sirkit::static_rectangle_add_rectangle_sum(additions, queries);
    assert((answers == std::vector<std::uint32_t>{20, 0}));
}
