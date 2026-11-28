#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_DIVISIBILITY_CHAINS
#define SIRKIT_DIVISIBILITY_CHAINS

namespace reinsirk::detail
{
constexpr std::uint32_t kDivisibilityChainMod = 998244353;

[[nodiscard]] constexpr std::uint32_t divisibility_chain_multiply(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kDivisibilityChainMod);
}

constexpr auto kPrimeExponentPartitions = []
{
    // uint64_tの正整数の素因数の指数は63以下。
    std::array<std::uint64_t, 64> count{};
    count[0] = 1;
    for (std::size_t part = 1; part < count.size(); ++part)
    {
        for (std::size_t sum = part; sum < count.size(); ++sum)
        {
            count[sum] += count[sum - part];
        }
    }
    return count;
}();
} // namespace reinsirk::detail

// p^eで終わる指数の非減少列を数える。e <= 30 なのでe!は法と互いに素。
[[nodiscard]] inline std::vector<std::uint32_t> divisibility_chain_end_counts(std::uint64_t length, const IntegerSieve &sieve)
{
    if (length == 0)
    {
        throw std::invalid_argument("divisibility_chain_end_counts: length must be positive");
    }
    const auto &least_prime_factor = sieve.least_prime_factor;
    const auto size = least_prime_factor.size();
    std::vector<std::uint32_t> result(size);
    if (size <= 1)
    {
        return result;
    }
    constexpr auto kMod = detail::kDivisibilityChainMod;
    const auto multiply = detail::divisibility_chain_multiply;
    std::array<std::uint32_t, 31> inverse{}, prime_power_ways{};
    inverse[1] = 1;
    prime_power_ways[0] = 1;
    prime_power_ways[1] = static_cast<std::uint32_t>(length % kMod);
    for (std::uint32_t exponent = 2; exponent < prime_power_ways.size(); ++exponent)
    {
        inverse[exponent] = kMod - multiply(kMod / exponent, inverse[kMod % exponent]);
        const auto term = (prime_power_ways[1] + exponent - 1) % kMod;
        prime_power_ways[exponent] = multiply(multiply(prime_power_ways[exponent - 1], term), inverse[exponent]);
    }
    result[1] = 1;
    std::vector<std::uint8_t> exponents(size);
    std::vector<std::uint32_t> other_factors(size, 1);
    for (std::size_t value = 2; value < size; ++value)
    {
        const auto prime = least_prime_factor[value];
        const auto previous = value / static_cast<std::size_t>(prime);
        if (least_prime_factor[previous] == prime)
        {
            exponents[value] = static_cast<std::uint8_t>(exponents[previous] + 1);
            other_factors[value] = other_factors[previous];
        }
        else
        {
            exponents[value] = 1;
            other_factors[value] = result[previous];
        }
        result[value] = multiply(other_factors[value], prime_power_ways[exponents[value]]);
    }
    return result;
}

[[nodiscard]] inline std::vector<std::uint32_t> divisibility_chain_end_counts(std::uint64_t length, int max_value)
{
    return divisibility_chain_end_counts(length, IntegerSieve(max_value));
}

// 素数ごとに指数の正の部分を分割し、長さの差は先頭を0で埋める。
[[nodiscard]] inline std::uint64_t divisibility_chain_product_count(std::uint64_t value)
{
    if (value == 1)
    {
        return 0;
    }
    std::uint64_t count = 1;
    for (const auto &[prime, exponent] : Factorize64(value))
    {
        count *= detail::kPrimeExponentPartitions[static_cast<std::size_t>(exponent)];
    }
    return count;
}
#endif
