// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Maximum_Weight_Window.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array<sirkit::WeightedWindowPoint, 3> points{{{2, 0, -2}, {10, 0, 3}, {12, 0, -4}}};
    const auto best = sirkit::maximum_weight_window(points, 9, 9);
    assert(best.weight == 3 && best.left == 3);
}
