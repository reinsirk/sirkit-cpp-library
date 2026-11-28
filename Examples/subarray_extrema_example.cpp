// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Subarray_Extrema.cpp"
#include <cassert>
#include <cstdint>
#include <functional>
#include <vector>

int main()
{
    const std::vector<int> values{3, 1, 2};
    const auto min = sirkit::subarray_extremum_sums_by_length<std::int64_t>(values);
    const auto max = sirkit::subarray_extremum_sums_by_length<std::int64_t>(values, std::greater<>{});
    assert((min == std::vector<std::int64_t>{6, 2, 1}));
    assert((max == std::vector<std::int64_t>{6, 5, 3}));
    const auto spans = sirkit::subarray_extremum_spans(values);
    assert(spans[1].left == 0 && spans[1].right == 3);
}
