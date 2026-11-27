// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/All_Furthest_Neighbors.cpp"
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    using Point = std::pair<std::int64_t, std::int64_t>;
    const std::vector<Point> square{{0, 0}, {4, 0}, {4, 4}, {0, 4}};
    assert((sirkit::all_furthest_neighbors(square) == std::vector<int>{2, 3, 0, 1}));
}
