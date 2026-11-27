// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Rectangle_Sum.cpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<sirkit::RectangleSumPoint> points{{0, 1, 5}, {0, 1, -2}, {2, 1, 10}};
    const std::vector<sirkit::RectangleSumQuery> queries{{0, 0, 2, 2}, {0, 0, 3, 2}};
    assert((sirkit::rectangle_sum(points, queries) == std::vector<std::int64_t>{3, 13}));
}
