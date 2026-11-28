// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Dynamic_Color_Component_Sum.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <utility>

int main()
{
    const std::array<std::int64_t, 3> weights{4, 6, 2};
    const std::array colors{0, 1, 0};
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
    sirkit::DynamicColorComponentSum tree(weights, colors, edges);
    assert(tree.component_sum(0) == 4);
    tree.toggle(1);
    assert(tree.component_sum(0) == 12);
    tree.add_weight(2, 5);
    assert(tree.component_sum(1) == 17);
}
