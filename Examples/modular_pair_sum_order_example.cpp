// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Modular_Pair_Sum_Order.cpp"
#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    const std::array<std::uint64_t, 2> left{2, 9}, right{1, 6};
    const sirkit::ModularPairSumOrder order(left, right, 10);
    assert(order.count_at_most(3) == 2);
    assert(order.kth_smallest(3) == 5);
}
