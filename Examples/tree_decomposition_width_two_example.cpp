// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Decomposition_Width_Two.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 0}};
    const auto result = sirkit::tree_decomposition_width_two(3, edges);
    assert(result && result->width == 2 && result->bags.size() == 3);
    assert(result->bags[0].size == 3 && result->tree_edges.size() == 2);
}
