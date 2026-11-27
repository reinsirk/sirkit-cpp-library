// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Topological_Order_Minimum_Inversions.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::array parent{-1, 0, 0};
    const std::array<std::uint64_t, 3> c{0, 2, 5}, d{0, 1, 1};
    const auto best = sirkit::minimum_inversion_topological_order(3, parent, c, d);
    assert(best.inversions == 2);
    assert((best.order == std::vector{0, 2, 1}));
}
