// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n, m;
    std::int64_t first;
    verify_in >> n >> m >> first;
    std::vector<std::uint32_t> samples(n);
    for (auto &value : samples)
    {
        verify_in >> value;
    }
    const auto result = evaluate_consecutive_polynomial(samples, first, m);
    for (std::size_t i = 0; i < m; ++i)
    {
        verify_out << result[i] << (i + 1 == m ? '\n' : ' ');
    }
}
