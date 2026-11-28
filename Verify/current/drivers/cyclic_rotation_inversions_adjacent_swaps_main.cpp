// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace
{
constexpr std::int64_t kMod = 998244353;

std::int64_t power(std::int64_t value, std::int64_t exponent)
{
    std::int64_t result = 1;
    while (exponent > 0)
    {
        if (exponent % 2 != 0)
        {
            result = result * value % kMod;
        }
        value = value * value % kMod;
        exponent /= 2;
    }
    return result;
}
}

int main()
{
    std::int64_t n{};
    int query_count{};
    verify_in >> n >> query_count;
    std::vector<int> values(static_cast<std::size_t>(n));
    for (auto &value : values)
    {
        verify_in >> value;
    }
    sirkit::CyclicInversionStatistics counts(values);
    const auto inv_n = power(n, kMod - 2);
    const auto total_pairs = n * (n - 1) / 2 % kMod;
    for (int query = 0; query < query_count; ++query)
    {
        std::size_t position{};
        std::int64_t steps{};
        verify_in >> position >> steps;
        counts.swap_adjacent(position - 1);

        const auto histories = power(n - 1, steps);
        const bool reversed = steps % 2 != 0;
        const auto sign = reversed ? kMod - 1 : 1;
        const auto common = (histories - sign + kMod) % kMod * inv_n % kMod;
        auto result = (common * (counts.rotation_inversion_sum() % kMod) + sign * (counts.inversion_count() % kMod)) % kMod;
        if (reversed)
        {
            result = (histories * total_pairs % kMod - result + kMod) % kMod;
        }
        verify_out << result << '\n';
    }
}
