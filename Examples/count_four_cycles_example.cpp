// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Count_Four_Cycles.cpp"
#include <cassert>

int main()
{
    const std::vector<std::pair<int, int>> edges{{0, 1}, {1, 2}, {2, 3}, {3, 0}, {3, 0}};
    assert((sirkit::count_four_cycles_per_edge(4, edges) == std::vector<std::uint64_t>{2, 2, 2, 1, 1}));
}
