// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    std::uint64_t left{}, right{};
    verify_in >> left >> right;
    const auto cover = sirkit::dyadic_interval_cover(left, right);
    verify_out << cover.size() << '\n';
    for (const auto &[l, r] : cover)
    {
        verify_out << l << ' ' << r << '\n';
    }
}
