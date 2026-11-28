// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{}, red{};
    verify_in >> n >> red;
    std::vector<std::int64_t> rewards(n);
    for (std::size_t i = 1; i < n; ++i)
    {
        std::int64_t weight{};
        verify_in >> weight;
        rewards[i - 1] += weight;
        rewards[i] += weight;
    }
    red = std::min(red, n - red);
    const sirkit::NonadjacentSelection profile(rewards, red);
    verify_out << profile.sums().back() << '\n';
}
