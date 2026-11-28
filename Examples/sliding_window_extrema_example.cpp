// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Sliding_Window_Extrema.cpp"

#include <cassert>
#include <functional>
#include <vector>

int main()
{
    const std::vector<int> values{4, 1, 1, 5};
    const auto positions = sirkit::sliding_window_argextrema(values, 2);
    assert((positions == std::vector<std::size_t>{1, 1, 2}));

    const std::vector<std::vector<int>> grid{{1, 5, 3}, {4, 2, 6}};
    const auto maxima = sirkit::sliding_window_extrema_2d(grid, 2, 2, std::greater<>{});
    assert((maxima == std::vector<std::vector<int>>{{5, 6}}));
}
