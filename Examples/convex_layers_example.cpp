// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Convex_Layers.cpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    using Point = std::pair<std::int64_t, std::int64_t>;
    const std::vector<Point> points{{0, 0}, {2, 0}, {2, 2}, {0, 2}, {1, 0}, {1, 1}};
    assert((sirkit::convex_layers(points) == std::vector<std::size_t>{1, 1, 1, 1, 1, 2}));
}
