// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Pair_Xor_Order.cpp"

#include <cassert>

int main()
{
    const sirkit::PairXorOrder<unsigned> order({0, 1, 2, 3, 4});
    assert(order.pair_count() == 10);
    assert(order.kth(6) == 4);
    assert(order.count_less(4) == 6);
}
