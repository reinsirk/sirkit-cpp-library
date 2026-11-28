// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    std::size_t n = 0;
    verify_in >> n;
    std::vector<sirkit::LISInterval> intervals(n);
    for (auto &[left, right] : intervals)
    {
        verify_in >> left >> right;
    }
    verify_out << sirkit::interval_lis_tails(intervals).size() << '\n';
}
