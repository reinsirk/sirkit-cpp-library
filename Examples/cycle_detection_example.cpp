// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Cycle_Detection.cpp"
#include <cassert>

int main()
{
    const std::vector<std::pair<int, int>> edges{{0, 1}, {1, 0}};
    const auto cycle = sirkit::find_cycle(2, edges);
    assert(cycle && cycle->edge_ids.size() == 2);
}
