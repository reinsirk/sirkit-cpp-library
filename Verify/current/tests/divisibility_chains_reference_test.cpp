// SPDX-License-Identifier: CC0-1.0
#include "Math/Divisibility_Chains.cpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;

std::uint64_t brute_product(std::uint64_t remaining, std::uint64_t previous)
{
    if (remaining == 1)
    {
        return 1;
    }
    std::uint64_t count = 0;
    for (auto next = previous; next <= remaining; next += previous)
    {
        if (next > 1 && remaining % next == 0)
        {
            count += brute_product(remaining / next, next);
        }
    }
    return count;
}

std::uint64_t partition_by_pentagonal_numbers(int exponent)
{
    std::array<std::int64_t, 64> ways{};
    ways[0] = 1;
    for (int total = 1; total <= exponent; ++total)
    {
        for (int j = 1; j * (3 * j - 1) / 2 <= total; ++j)
        {
            const auto sign = j % 2 == 1 ? 1 : -1;
            ways[total] += sign * ways[total - j * (3 * j - 1) / 2];
            const auto other = total - j * (3 * j + 1) / 2;
            if (other >= 0)
            {
                ways[total] += sign * ways[other];
            }
        }
    }
    return static_cast<std::uint64_t>(ways[exponent]);
}
}

int main()
{
    for (int max_value = 0; max_value <= 65; ++max_value)
    {
        const sirkit::IntegerSieve sieve(max_value);
        auto want = std::vector<std::uint32_t>(static_cast<std::size_t>(max_value) + 1, 1);
        want[0] = 0;
        for (std::uint64_t length = 1; length <= 12; ++length)
        {
            if (sirkit::divisibility_chain_end_counts(length, sieve) != want ||
                sirkit::divisibility_chain_end_counts(length, max_value) != want)
            {
                return 1;
            }
            std::vector<std::uint32_t> next(want.size());
            for (int a = 1; a <= max_value; ++a)
            {
                for (int b = a; b <= max_value; b += a)
                {
                    next[b] = (next[b] + want[a]) % kMod;
                }
            }
            want = std::move(next);
        }
    }
    for (std::uint64_t value = 2; value <= 650; ++value)
    {
        if (sirkit::divisibility_chain_product_count(value) != brute_product(value, 1))
        {
            return 2;
        }
    }
    constexpr auto kLength = std::numeric_limits<std::uint64_t>::max();
    const auto high_length = sirkit::divisibility_chain_end_counts(kLength, 10);
    const auto zero_residue = sirkit::divisibility_chain_end_counts(kMod, 10);
    constexpr auto kReduced = kLength % kMod;
    if (high_length[1] != 1 || high_length[4] != (kReduced * (kReduced + 1) / 2) % kMod ||
        high_length[6] != kReduced * kReduced % kMod || zero_residue != std::vector<std::uint32_t>{0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0})
    {
        return 3;
    }
    if (sirkit::divisibility_chain_product_count(1) != 0 ||
        sirkit::divisibility_chain_product_count(std::uint64_t{1} << 63) != partition_by_pentagonal_numbers(63) ||
        sirkit::divisibility_chain_product_count(17ULL * 17ULL * (std::uint64_t{1} << 39)) != 2 * partition_by_pentagonal_numbers(39))
    {
        return 4;
    }
    try
    {
        static_cast<void>(sirkit::divisibility_chain_end_counts(0, 5));
        return 5;
    }
    catch (const std::invalid_argument &)
    {
    }
    try
    {
        const sirkit::IntegerSieve sieve(10);
        static_cast<void>(sirkit::divisibility_chain_end_counts(0, sieve));
        return 6;
    }
    catch (const std::invalid_argument &)
    {
    }
    try
    {
        static_cast<void>(sirkit::divisibility_chain_product_count(0));
        return 7;
    }
    catch (const std::domain_error &)
    {
    }
}
