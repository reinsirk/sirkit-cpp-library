// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Grid_Connectivity_Frontier.cpp"

#include <cassert>

int main()
{
    assert(sirkit::grid_corner_connected_colorings(2, 2) == 3);
    const sirkit::GridConnectivityFrontier grid(2);
    auto first = grid.place(0, 0, true, true);
    auto next = grid.place(first.state, 1, true);
    assert(grid.label(next.state, 1) == grid.kMarkedGroup);
}
