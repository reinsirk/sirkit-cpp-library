// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Pair_Sum_Order.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array<std::int64_t, 4> values{3, 1, 2, 2};
    const sirkit::PairSumOrder order(values);
    assert(order.count_equal(4) == 2);
    assert(order.kth_smallest(1) == 3);
    assert(order.kth_largest(2) == 5);
}
