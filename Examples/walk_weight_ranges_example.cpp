// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Walk_Weight_Ranges.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array edges{sirkit::WalkWeightEdge{0, 1, -3}, sirkit::WalkWeightEdge{0, 1, 5},
                           sirkit::WalkWeightEdge{1, 2, 2}, sirkit::WalkWeightEdge{2, 2, 1}};
    const auto range = sirkit::walk_weight_ranges(4, edges, 0);
    assert(range[1].minimum == -3 && range[1].maximum == 5);
    assert(range[2].unbounded && !range[3].reachable);
}
