// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    std::uint64_t wanted{};
    verify_in >> n >> wanted;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const sirkit::PairSumOrder order(values, values);
    const auto threshold = *order.kth_largest(wanted);
    const auto sorted = order.sorted_left();
    std::vector<std::int64_t> prefix(n + 1);
    for (std::size_t i = 0; i < n; ++i)
    {
        prefix[i + 1] = prefix[i] + sorted[i];
    }
    std::size_t begin = n;
    std::uint64_t greater_count = 0;
    std::int64_t greater_sum = 0;
    for (const auto value : sorted)
    {
        while (begin != 0 && value + sorted[begin - 1] > threshold)
        {
            --begin;
        }
        const auto remaining = n - begin;
        greater_count += remaining;
        greater_sum += static_cast<std::int64_t>(remaining) * value + prefix[n] - prefix[begin];
    }
    const auto answer = greater_sum + static_cast<std::int64_t>(wanted - greater_count) * threshold;
    verify_out << answer << '\n';
}
