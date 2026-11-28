// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Grid_Slide_Distances.cpp"

#include <array>
#include <cassert>
#include <string>

int main()
{
    const std::array<std::string, 3> grid{"...", "...", "..."};
    assert(sirkit::grid_slide_distance(grid, {0, 0}, {2, 2}, sirkit::GridSlideAxes::diagonal) == 1);
    assert(sirkit::grid_slide_distance(grid, {0, 0}, {2, 2}) == 2);
    const auto distances = sirkit::grid_slide_distances(grid, {0, 0}, sirkit::GridSlideAxes::diagonal);
    assert(distances[1] == -1 && distances[8] == 1);
}
