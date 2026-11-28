// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Adjacent_Swap_Distance.cpp"
#include <cassert>
#include <string_view>

int main()
{
    const std::string_view from = "aabb", to = "baba";
    const auto order = sirkit::adjacent_swap_permutation(from, to);
    assert(order && (*order == std::vector<std::size_t>{1, 3, 0, 2}));
    const auto swaps = sirkit::adjacent_swap_distance(from, to);
    assert(swaps && *swaps == 3);
}
