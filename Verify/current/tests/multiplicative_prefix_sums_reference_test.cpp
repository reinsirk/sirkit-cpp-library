// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Multiplicative_Prefix_Sums.cpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
std::uint64_t Direct(unsigned n, unsigned a, unsigned b)
{
    std::uint64_t value = 1;
    for (unsigned p = 2; p <= n / p; ++p)
    {
        unsigned exponent = 0;
        while (n % p == 0)
        {
            ++exponent;
            n /= p;
        }
        if (exponent != 0)
        {
            value *= std::uint64_t{a} * exponent + std::uint64_t{b} * p;
        }
    }
    if (n > 1)
    {
        value *= a + std::uint64_t{b} * n;
    }
    return value;
}

void Check(unsigned n, unsigned a, unsigned b)
{
    using u64 = std::uint64_t;
    const auto is_prime = [](unsigned value)
    {
        if (value < 2)
        {
            return false;
        }
        for (unsigned p = 2; p <= value / p; ++p)
        {
            if (value % p == 0)
            {
                return false;
            }
        }
        return true;
    };
    std::vector<u64> count(n + 1), prime_weight(n + 1), expected(n + 1);
    for (unsigned i = 1; i <= n; ++i)
    {
        count[i] = count[i - 1];
        prime_weight[i] = prime_weight[i - 1];
        if (is_prime(i))
        {
            ++count[i];
            prime_weight[i] += i;
        }
        expected[i] = expected[i - 1] + Direct(i, a, b);
    }
    const sirkit::MultiplicativePrefixSums sieve(n);
    auto counts = sieve.prime_prefix_sums([](u64)
                                          {
                                              return u64{1};
                                          },
                                          [](u64 x)
                                          {
                                              return x;
                                          });
    const auto primes = sieve.prime_prefix_sums([](u64 p)
                                                {
                                                    return p;
                                                },
                                                [](u64 x)
                                                {
                                                    return x * (x + 1) / 2;
                                                });
    std::vector<unsigned> coordinates;
    for (unsigned divisor = n; divisor >= 1; --divisor)
    {
        const auto value = n / divisor;
        if (coordinates.empty() || coordinates.back() != value)
        {
            coordinates.push_back(value);
        }
    }
    assert(sieve.size() == coordinates.size());
    for (std::size_t i = 0; i < coordinates.size(); ++i)
    {
        const auto coordinate = sieve.quotient(i);
        assert(coordinate == coordinates[i]);
        assert(counts[i] == count[coordinate]);
        assert(primes[i] == prime_weight[coordinate]);
        counts[i] = counts[i] * a + primes[i] * b;
    }
    const auto at_power = [&](u64 p, unsigned exponent)
    {
        return std::uint64_t{a} * exponent + p * b;
    };
    assert(sieve.prefix_sum(counts, at_power) == expected.back());
    const auto answer = sieve.prefix_sums(std::move(counts), at_power);
    for (std::size_t i = 0; i < coordinates.size(); ++i)
    {
        assert(answer[i] == expected[coordinates[i]]);
    }
}

std::uint64_t Triangular(std::uint64_t value)
{
    return (value & 1) != 0 ? value * (value / 2 + 1) : (value / 2) * (value + 1);
}

void CheckDividendWidth()
{
    using u64 = std::uint64_t;
    constexpr u64 kN = static_cast<u64>(std::numeric_limits<std::uint32_t>::max()) + 20;
    const sirkit::MultiplicativePrefixSums sieve(kN);
    const auto at_power = [](u64 prime, unsigned exponent)
    {
        u64 result = 1;
        for (unsigned i = 0; i < exponent; ++i)
        {
            result *= prime;
        }
        return result;
    };
    auto prime_sums = sieve.prime_prefix_sums([](u64 prime)
                                              {
                                                  return prime;
                                              },
                                              Triangular);
    assert(sieve.prefix_sum(prime_sums, at_power) == Triangular(kN));
    const auto all = sieve.prefix_sums(std::move(prime_sums), at_power);
    for (std::size_t i = 0; i < all.size(); ++i)
    {
        assert(all[i] == Triangular(sieve.quotient(i)));
    }
}
}

int main()
{
    for (unsigned n : {1u, 2u, 4u, 17u, 29u, 64u, 100u, 1023u, 1024u, 1025u, 15551u})
    {
        Check(n, n % 5, n % 7);
    }
    Check(4097, std::numeric_limits<unsigned>::max() - 2, std::numeric_limits<unsigned>::max() - 3);
    CheckDividendWidth();
#ifndef SKIP_RUNTIME_CHECKS
    using Sieve = sirkit::MultiplicativePrefixSums;
    try
    {
        static_cast<void>(Sieve(0));
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
    const Sieve sieve(7);
    try
    {
        static_cast<void>(sieve.quotient(sieve.size()));
        assert(false);
    }
    catch (const std::out_of_range &)
    {
    }
    try
    {
        static_cast<void>(sieve.prefix_sums<std::uint64_t>({}, [](std::uint64_t, unsigned)
                                                           {
                                                               return std::uint64_t{1};
                                                           }));
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#endif
}
