// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Subarray_Median_Order.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array values{10, 30, 20};
    assert(sirkit::count_subarray_medians_at_least(values, 30) == 3);
    assert(sirkit::kth_subarray_median(values, 4) == 30);
    assert(sirkit::kth_subarray_median(values, 4, sirkit::SubarrayMedianSide::lower) == 20);
}
