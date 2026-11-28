// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    const int doubled = 2 * n - 1;
    std::vector<std::int64_t> prefix(static_cast<std::size_t>(doubled + 1));
    for (int i = 0; i < n; ++i)
    {
        std::int64_t value{};
        verify_in >> value;
        prefix[static_cast<std::size_t>(i + 1)] = prefix[static_cast<std::size_t>(i)] + value;
    }
    const auto total = prefix[static_cast<std::size_t>(n)];
    for (int i = n + 1; i <= doubled; ++i)
    {
        prefix[static_cast<std::size_t>(i)] = total + prefix[static_cast<std::size_t>(i - n)];
    }
    const sirkit::KnuthIntervalDP dp(doubled, n, [&](int left, int right)
    {
        return prefix[static_cast<std::size_t>(right)] - prefix[static_cast<std::size_t>(left)];
    });
    auto answer = dp.get(0, n);
    for (int i = 1; i < n; ++i)
    {
        answer = std::min(answer, dp.get(i, i + n));
    }
    verify_out << answer << '\n';
}
