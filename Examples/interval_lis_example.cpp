// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Interval_LIS.cpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::array intervals{
        sirkit::LISInterval{3, 5},
        sirkit::LISInterval{1, 4},
        sirkit::LISInterval{2, 7},
    };
    const auto tails = sirkit::interval_lis_tails(intervals);
    assert((tails == std::vector<std::int64_t>{1, 2, 5}));
}
