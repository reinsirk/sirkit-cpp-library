// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Knuth_Interval_DP.cpp"

#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    const std::array<std::int64_t, 4> prefix{0, 1, 3, 6};
    const sirkit::KnuthIntervalDP dp(3, [&](int l, int r)
    {
        return prefix[r] - prefix[l];
    });
    assert(dp.get(0, 3) == 9 && dp.split(0, 3) == 2);
    assert(!dp.split(1, 2));
}
