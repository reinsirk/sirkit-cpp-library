// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Minkowski_Sum.cpp"
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    using Point = std::pair<std::int64_t, std::int64_t>;
    const std::vector<Point> square{{0, 0}, {2, 0}, {2, 2}, {0, 2}};
    const std::vector<Point> segment{{0, 0}, {1, 0}};
    const std::vector<Point> rectangle{{0, 0}, {3, 0}, {3, 2}, {0, 2}};
    assert(sirkit::minkowski_sum(square, segment) == rectangle);
}
