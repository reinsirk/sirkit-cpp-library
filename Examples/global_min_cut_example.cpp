// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Global_Min_Cut.cpp"
#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::GlobalMinCutEdge> edges{{0, 1, 7}, {1, 2, 2}};
    const auto cut = sirkit::global_min_cut(3, edges);
    assert(cut && cut->weight == 2);
    assert(!cut->side.empty() && cut->side.size() < 3);
}
