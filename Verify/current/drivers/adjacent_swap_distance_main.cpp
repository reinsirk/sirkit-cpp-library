// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n = 0;
    verify_in >> n;
    std::vector<std::int64_t> from(n), to(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        verify_in >> from[i];
        from[i] += static_cast<std::int64_t>(i);
    }
    for (std::size_t i = 0; i < n; ++i)
    {
        verify_in >> to[i];
        to[i] += static_cast<std::int64_t>(i);
    }
    const auto swaps = sirkit::adjacent_swap_distance(from, to);
    if (swaps)
    {
        verify_out << *swaps << '\n';
    }
    else
    {
        verify_out << "-1\n";
    }
}
