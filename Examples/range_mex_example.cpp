// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Range_Mex.cpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::array<std::int64_t, 4> values{0, 2, 1, 0};
    const std::array queries{sirkit::RangeMexQuery{1, 3}, sirkit::RangeMexQuery{0, 3}};
    assert((sirkit::range_mex(values, queries) == std::vector<std::size_t>{0, 3}));
    assert((sirkit::subarray_mex_histogram(values) == std::vector<std::uint64_t>{3, 3, 1, 3, 0}));
}
