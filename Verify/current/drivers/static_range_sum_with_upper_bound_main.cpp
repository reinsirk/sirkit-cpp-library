// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const WeightedWaveletMatrix<> matrix(values, values);
    while (q-- > 0)
    {
        int left = 0, right = 0;
        std::int64_t limit = 0;
        verify_in >> left >> right >> limit;
        const auto [count, sum] = matrix.count_sum_less_equal(left, right, limit);
        verify_out << count << ' ' << sum << '\n';
    }
}
