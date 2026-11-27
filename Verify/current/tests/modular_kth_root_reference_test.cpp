// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Modular_Kth_Root.cpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
std::uint32_t Power(std::uint64_t x, std::uint64_t k, std::uint32_t p)
{
    x %= p;
    std::uint64_t result = 1;
    for (; k != 0; k /= 2)
    {
        if (k % 2 != 0)
        {
            result = result * x % p;
        }
        x = x * x % p;
    }
    return static_cast<std::uint32_t>(result);
}

bool IsPrime(std::uint32_t n)
{
    for (std::uint32_t d = 2; d * d <= n; ++d)
    {
        if (n % d == 0)
        {
            return false;
        }
    }
    return n > 1;
}
}

int main()
{
    using sirkit::modular_kth_root;
    assert(modular_kth_root(0, 1, 2) == 0);
    assert(!modular_kth_root(0, 0, 2));
    assert(modular_kth_root(4, 0, 2) == 0);
    assert(modular_kth_root(4, 9, 2) == 1);
    assert(modular_kth_root(5, 17, 13) == 10);
    assert(!modular_kth_root(12, 2, 13));

    for (std::uint32_t p = 2; p <= 137; ++p)
    {
        if (!IsPrime(p))
        {
            continue;
        }
        for (std::uint32_t k = 0; k <= 2 * p + 1; ++k)
        {
            std::vector<bool> possible(p);
            for (std::uint32_t x = 0; x < p; ++x)
            {
                possible[Power(x, k, p)] = true;
            }
            for (std::uint32_t y = 0; y < p; ++y)
            {
                const auto root = modular_kth_root(k, y, p);
                assert(root.has_value() == possible[y]);
                assert(!root || (*root < p && Power(*root, k, p) == y));
            }
        }
    }

    struct LargeCase
    {
        std::uint32_t p;
        std::uint64_t k;
    };
    constexpr auto kMaxExponent = std::numeric_limits<std::uint64_t>::max();
    constexpr LargeCase cases[]{
        {4294967291U, kMaxExponent},
        {4294967291U, 19},
        {990612677U, 15737},
        {3616924253U, 967},
        {999999503U, 499999751U},
        {998244353U, 2},
        {998244353U, 32},
        {998244353U, 7U << 20},
        {2013265921U, 1U << 26},
    };
    std::mt19937_64 rng(10625711);
    for (const auto &[p, k] : cases)
    {
        for (int i = 0; i < 160; ++i)
        {
            const auto expected = Power(rng(), k, p);
            const auto result = modular_kth_root(k, expected, p);
            assert(result && *result < p && Power(*result, k, p) == expected);
            const auto y = rng() % p;
            const auto root = modular_kth_root(k, y, p);
            const auto order = (p - 1) / std::gcd(k, std::uint64_t{p - 1});
            const bool possible = y == 0 || Power(y, order, p) == 1;
            assert(root.has_value() == possible);
            assert(!root || (*root < p && Power(*root, k, p) == y));
        }
    }
    constexpr auto kSafePrime = 4294967087U;
    constexpr auto kSafeFactor = 2147483543U;
    const auto full_trial = modular_kth_root(kSafeFactor, kSafePrime - 1, kSafePrime);
    assert(full_trial && Power(*full_trial, kSafeFactor, kSafePrime) == kSafePrime - 1);

    constexpr auto kPrime = 4294967291U;
    const auto reduced = modular_kth_root(kMaxExponent, kMaxExponent, kPrime);
    assert(!reduced || Power(*reduced, kMaxExponent, kPrime) == kMaxExponent % kPrime);

    for (const std::uint32_t bad : {0, 1})
    {
        try
        {
            (void)modular_kth_root(2, 1, bad);
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    try
    {
        (void)modular_kth_root(1, 2, 4);
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#endif
}
