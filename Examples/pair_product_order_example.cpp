// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Pair_Product_Order.cpp"
#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    const std::array<std::int64_t, 4> values{3, 3, -4, -2};
    const sirkit::PairProductOrder products(values);
    assert(products.kth_smallest(3) == -6);
    assert(products.count_at_most(-6) == 4);
}
