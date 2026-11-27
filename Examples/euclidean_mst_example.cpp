// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Euclidean_MST.cpp"
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    using Point = std::pair<std::int64_t, std::int64_t>;
    const std::vector<Point> points{{0, 0}, {2, 0}, {0, 1}};
    const auto tree = sirkit::euclidean_mst(points);
    assert(tree.size() == 2);
    assert(tree[0].distance_squared == 1);
    assert(tree[1].distance_squared == 4);
}
