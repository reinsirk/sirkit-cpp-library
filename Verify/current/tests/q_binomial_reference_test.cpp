// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Combination/Q_Binomial.cpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace
{
void CheckSmall(std::uint32_t prime)
{
    const auto limit = prime - 1;
    for (std::uint32_t q = 0; q < prime; ++q)
    {
        const sirkit::PrimeQBinomial coefficients(prime, q, limit);
        std::vector<std::uint32_t> powers(prime, 1), row(prime, 0);
        for (std::uint32_t i = 1; i < prime; ++i)
        {
            powers[i] = powers[i - 1] * q % prime;
        }
        row[0] = 1;
        for (std::uint32_t n = 0; n <= limit; ++n)
        {
            for (std::uint32_t k = n; k > 0; --k)
            {
                row[k] = (row[k] + powers[n - k] * row[k - 1]) % prime;
            }
            for (std::uint32_t k = 0; k <= n; ++k)
            {
                assert(coefficients.COM(n, k) == row[k]);
            }
            assert(coefficients.COM(n, n + 1) == 0);
        }
    }
}
}

int main()
{
    for (const auto p : {2u, 3u, 5u, 7u, 11u, 13u, 17u, 19u, 29u, 41u, 59u, 61u})
    {
        CheckSmall(p);
    }
    constexpr std::uint32_t kPrime = 4294967291u;
    constexpr std::uint32_t kLimit = 100000;
    const sirkit::PrimeQBinomial large(kPrime, 2, kLimit);
    std::uint64_t power = 1, sum = 0;
    for (std::uint32_t n = 1; n <= kLimit; ++n)
    {
        sum = (sum + power) % kPrime;
        power = power * 2 % kPrime;
    }
    assert(large.COM(kLimit, 0) == 1);
    assert(large.COM(kLimit, 1) == sum);
    assert(large.COM(kLimit, kLimit - 1) == sum);
    assert(large.COM(kLimit, kLimit) == 1);
    assert(sirkit::PrimeQBinomial(13, 3, 0).COM(0, 0) == 1);
    assert(sirkit::PrimeQBinomial(13, 1, 0).COM(0, 0) == 1);
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto p : {167772161u, 469762049u, 754974721u, 998244353u, 1000000007u,
                         1000000009u, 1004535809u, 1224736769u, 2013265921u})
    {
        assert(sirkit::PrimeQBinomial(p, 2, 0).COM(0, 0) == 1);
    }
    for (const auto &invalid : {std::array{7u, 7u, 0u}, std::array{5u, 0u, 5u}, std::array{1u, 0u, 0u},
                                std::array{4u, 0u, 0u}, std::array{9u, 0u, 0u}, std::array{25u, 0u, 0u},
                                std::array{49u, 0u, 0u}, std::array{121u, 0u, 0u}, std::array{169u, 0u, 0u}})
    {
        try
        {
            const sirkit::PrimeQBinomial unused(invalid[0], invalid[1], invalid[2]);
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
    try
    {
        static_cast<void>(large.COM(kLimit + 1, 0));
        assert(false);
    }
    catch (const std::out_of_range &)
    {
    }
#endif
}
