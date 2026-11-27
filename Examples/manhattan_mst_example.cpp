// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Manhattan_MST.cpp"
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::pair<std::int64_t, std::int64_t>> points{{0, 0}, {1, 2}, {1, 0}};
    const auto tree = sirkit::manhattan_mst(points);
    assert(tree.cost == 3);
    assert(tree.edges.size() == 2);
}
