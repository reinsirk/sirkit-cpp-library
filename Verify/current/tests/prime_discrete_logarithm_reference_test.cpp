// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Prime_Discrete_Logarithm.cpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace
{
std::vector<std::uint32_t> Powers(std::uint32_t prime, std::uint32_t base)
{
    std::vector<std::uint32_t> values;
    std::uint32_t value = 1;
    do
    {
        values.push_back(value);
        value = static_cast<std::uint32_t>(static_cast<std::uint64_t>(value) * base % prime);
    } while (value != 1);
    return values;
}

void CheckSmall(std::uint32_t prime)
{
    for (std::uint32_t base = 1; base < prime; ++base)
    {
        const auto expected = Powers(prime, base);
        if (expected.size() != prime - 1)
        {
#ifndef SKIP_RUNTIME_CHECKS
            try
            {
                const sirkit::PrimeDiscreteLogarithm invalid(prime, base);
                assert(false);
            }
            catch (const std::invalid_argument &)
            {
            }
#endif
            continue;
        }
        for (const auto count : {0u, 1u, 10u, 200u})
        {
            const sirkit::PrimeDiscreteLogarithm calculator(prime, base, count);
            for (std::uint32_t exponent = 0; exponent < prime - 1; ++exponent)
            {
                assert(calculator.log(expected[exponent]) == exponent);
            }
        }
    }
}

std::uint32_t ReferencePower(std::uint64_t base, std::uint32_t exponent, std::uint32_t prime)
{
    std::uint64_t value = 1;
    while (exponent != 0)
    {
        if ((exponent & 1u) != 0)
        {
            value = value * base % prime;
        }
        base = base * base % prime;
        exponent >>= 1;
    }
    return static_cast<std::uint32_t>(value);
}
}

int main()
{
    for (const auto prime : {2u, 3u, 5u, 7u, 11u, 13u, 17u, 19u, 29u, 41u, 59u, 61u, 101u})
    {
        CheckSmall(prime);
    }
    constexpr auto kPrime = 65537u;
    constexpr auto kRoot = 3u;
    const auto expected = Powers(kPrime, kRoot);
    assert(expected.size() == kPrime - 1);
    const sirkit::PrimeDiscreteLogarithm sparse(kPrime, kRoot, 1);
    const sirkit::PrimeDiscreteLogarithm dense(kPrime, kRoot, 4096);
    std::mt19937 random(214);
    for (int i = 0; i < 1024; ++i)
    {
        const auto exponent = random() % expected.size();
        assert(sparse.log(expected[exponent]) == exponent);
        assert(dense.log(expected[exponent]) == exponent);
    }
    assert(sparse.log(1) == 0);
    assert(sparse.log(expected.back()) == expected.size() - 1);
    assert(dense.log(expected.back()) == expected.size() - 1);
    constexpr std::uint32_t kLargePrime = 4294967291u;
    const sirkit::PrimeDiscreteLogarithm large(kLargePrime, 2);
    for (const auto exponent : {0u, 1u, 61729u, 65536u, 1234567890u, kLargePrime - 2})
    {
        assert(large.log(ReferencePower(2, exponent, kLargePrime)) == exponent);
    }
#ifndef SKIP_RUNTIME_CHECKS
    assert(!sirkit::detail::is_prime_u32(1));
    for (const auto &pair : {std::array{7u, 0u}, std::array{7u, 7u}, std::array{1u, 1u}, std::array{4u, 1u},
                             std::array{9u, 1u}, std::array{25u, 1u}, std::array{49u, 1u}, std::array{121u, 1u},
                             std::array{169u, 1u}})
    {
        try
        {
            const sirkit::PrimeDiscreteLogarithm invalid(pair[0], pair[1]);
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
    for (const auto value : {0u, kPrime})
    {
        try
        {
            static_cast<void>(sparse.log(value));
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
    for (const auto prime : {167772161u, 469762049u, 754974721u, 998244353u, 1000000007u,
                             1000000009u, 1004535809u, 1224736769u, 2013265921u})
    {
        try
        {
            const sirkit::PrimeDiscreteLogarithm invalid(prime, 1);
            assert(false);
        }
        catch (const std::invalid_argument &error)
        {
            assert(error.what() == std::string_view("PrimeDiscreteLogarithm: primitive root required"));
        }
    }
#endif
}
