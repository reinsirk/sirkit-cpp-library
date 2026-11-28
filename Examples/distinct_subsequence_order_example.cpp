// SPDX-License-Identifier: CC0-1.0
#include "../String/Distinct_Subsequence_Order.cpp"
#include <cassert>

int main()
{
    const sirkit::DistinctSubsequenceOrder order("aba");
    assert(order.count_distinct() == 6);
    assert(order.kth_smallest(4) == "aba");
    assert(order.rank_of("ba") == 6);
    assert(order.prefix_count("a") == 4);
}
