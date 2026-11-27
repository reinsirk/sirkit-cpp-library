// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n;
    verify_in >> n;
    std::vector<std::vector<std::int64_t>> cost(n, std::vector<std::int64_t>(n));
    for (auto &row : cost)
    {
        for (auto &value : row)
        {
            verify_in >> value;
        }
    }
    const auto result = minimum_assignment(cost);
    verify_out << result.cost << '\n';
    for (int i = 0; i < n; ++i)
    {
        verify_out << result.column[i] << (i + 1 == n ? '\n' : ' ');
    }
}
