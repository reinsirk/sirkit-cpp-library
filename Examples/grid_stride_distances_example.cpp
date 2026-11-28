// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Grid_Stride_Distances.cpp"

#include <array>
#include <cassert>
#include <string>

int main()
{
    const std::array<std::string, 2> grid{"...", ".#."};
    assert(sirkit::grid_stride_distance(grid, {1, 0}, {1, 2}, 3) == 3);
    const auto all = sirkit::grid_stride_distances(grid, {0, 0}, 3);
    assert(all[2] == 1 && all[4] == -1);
}
