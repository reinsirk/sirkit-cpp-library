// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <string>
#include <vector>

int main()
{
    int n{}, x1{}, y1{}, x2{}, y2{};
    verify_in >> n >> x1 >> y1 >> x2 >> y2;
    std::vector<std::string> grid(n);
    for (auto &row : grid)
    {
        verify_in >> row;
    }
    verify_out << sirkit::grid_slide_distance(grid, {x1 - 1, y1 - 1}, {x2 - 1, y2 - 1},
                                              sirkit::GridSlideAxes::diagonal)
               << '\n';
}
