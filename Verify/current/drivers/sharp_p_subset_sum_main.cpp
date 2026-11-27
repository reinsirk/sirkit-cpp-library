// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    std::size_t n, limit;
    verify_in >> n >> limit;
    std::vector<std::size_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const auto counts = subset_sum_counts(values, limit);
    for (std::size_t i = 1; i <= limit; ++i)
    {
        verify_out << counts[i] << (i == limit ? '\n' : ' ');
    }
}
