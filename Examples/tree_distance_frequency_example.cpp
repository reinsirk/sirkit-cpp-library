// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Distance_Frequency.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const sirkit::Graph path{{1}, {0, 2}, {1}};
    const auto count = sirkit::tree_distance_frequency(path);
    assert((count == std::vector<std::int64_t>{0, 2, 1}));
}
