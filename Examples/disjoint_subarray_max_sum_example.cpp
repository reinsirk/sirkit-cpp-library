// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Disjoint_Subarray_Max_Sum.cpp"
#include <cassert>
#include <vector>

int main()
{
    const std::vector<int> values{4, -5, 6, 2};
    const sirkit::DisjointSubarrayMaxSum data(values);
    assert(data.max_sum(2) == 12);
    const auto ranges = data.intervals(2).value();
    assert(ranges.size() == 2);
    assert(ranges[0].first == 0 && ranges[0].second == 1);
    assert(ranges[1].first == 2 && ranges[1].second == 4);
}
