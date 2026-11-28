// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

int main()
{
    int n{}, red{};
    verify_in >> n >> red;
    std::vector<std::int64_t> rewards(n);
    for (int i = 1; i < n; ++i)
    {
        std::int64_t value{};
        verify_in >> value;
        rewards[i - 1] += value;
        rewards[i] += value;
    }
    verify_out << sirkit::maximum_path_matching_weight<std::int64_t>(
                      rewards, std::min(red, n - red))
               << '\n';
}
