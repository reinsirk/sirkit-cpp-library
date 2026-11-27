// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Exponential_Polynomial_Sum.cpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
constexpr std::uint64_t kMod = 998244353;

std::uint64_t power(std::uint64_t base, std::uint64_t exponent)
{
    if (exponent == 0)
    {
        return 1;
    }
    const auto half = power(base, exponent / 2);
    const auto square = half * half % kMod;
    return exponent % 2 ? square * (base % kMod) % kMod : square;
}

std::uint64_t reference_limit(std::uint64_t ratio, unsigned degree)
{
    std::vector<std::uint64_t> stirling(degree + 1);
    stirling[0] = 1;
    for (unsigned i = 1; i <= degree; ++i)
    {
        for (unsigned j = i; j > 0; --j)
        {
            stirling[j] = (stirling[j - 1] + j * stirling[j]) % kMod;
        }
        stirling[0] = 0;
    }
    const auto inverse = power((1 + kMod - ratio) % kMod, kMod - 2);
    const auto factor = ratio * inverse % kMod;
    std::uint64_t weighted_factorial = inverse, answer = 0;
    for (unsigned j = 0; j <= degree; ++j)
    {
        answer = (answer + stirling[j] * weighted_factorial) % kMod;
        weighted_factorial = weighted_factorial * factor % kMod * (j + 1) % kMod;
    }
    return answer;
}

std::uint64_t reference_linear_sum(std::uint64_t ratio, std::uint64_t n)
{
    if (n == 0)
    {
        return 0;
    }
    if (ratio == 1)
    {
        return (n % kMod) * ((n - 1) % kMod) % kMod * ((kMod + 1) / 2) % kMod;
    }
    const auto p = power(ratio, n);
    const auto inverse = power((1 + kMod - ratio) % kMod, kMod - 2);
    const auto numerator = (ratio + kMod - p * (n % kMod) % kMod + (n - 1) % kMod * p % kMod * ratio % kMod) % kMod;
    return numerator * inverse % kMod * inverse % kMod;
}
}

int main()
{
    using sirkit::sum_exponential_polynomial;
    using sirkit::sum_exponential_polynomial_limit;
    using sirkit::sum_exponential_powers;
    using sirkit::sum_exponential_powers_limit;
    const std::array<std::uint32_t, 2> linear{7, 8};
    assert(sum_exponential_polynomial({}, 2, 100) == 0);
    assert(sum_exponential_polynomial(linear, 2, 0) == 0);
    assert(sum_exponential_polynomial(linear, kMod, 100) == 7);
    assert(sum_exponential_polynomial_limit({}, 2) == 0);
    assert(sum_exponential_polynomial_limit(linear, 0) == 7);
    assert(!sum_exponential_polynomial_limit(linear, kMod + 1));
    assert(!sum_exponential_powers_limit(1, 8));
    assert(sum_exponential_powers(0, 0, 2) == 1);
    assert(sum_exponential_powers(0, 1, 2) == 0);
    assert(sum_exponential_powers(0, 0, 0) == 0);
    const std::array<std::uint64_t, 4> large{1, kMod + 1, 2 * kMod + 3, std::numeric_limits<std::uint64_t>::max()};
    for (const auto n : large)
    {
        for (const auto r : {std::uint64_t{0}, std::uint64_t{1}, std::uint64_t{2}, kMod - 1})
        {
            assert(sum_exponential_powers(r, 1, n) == reference_linear_sum(r, n));
        }
    }
    for (unsigned degree = 0; degree <= 32; ++degree)
    {
        for (const auto ratio : {std::uint64_t{0}, std::uint64_t{2}, std::uint64_t{13}, kMod - 1})
        {
            assert(sum_exponential_powers_limit(ratio, degree) == reference_limit(ratio, degree));
        }
    }
    std::mt19937 rng(8301);
    for (int i = 0; i < 760; ++i)
    {
        const unsigned degree = rng() % 25;
        const unsigned length = rng() % 170;
        const std::uint64_t ratio = i % 4 == 0 ? i % 3 : rng() % kMod;
        std::uint64_t sum = 0, ratio_power = 1;
        for (unsigned k = 0; k < length; ++k)
        {
            sum = (sum + ratio_power * power(k, degree)) % kMod;
            ratio_power = ratio_power * ratio % kMod;
        }
        assert(sum_exponential_powers(ratio + kMod, degree, length) == sum);
    }
    for (int i = 0; i < 200; ++i)
    {
        const std::uint64_t ratio = i % 6 + 2;
        const std::uint64_t length = rng() % 63;
        std::vector<std::uint32_t> coefficients(rng() % 9 + 1), samples(coefficients.size());
        std::uint64_t limit = 0;
        for (unsigned j = 0; j < coefficients.size(); ++j)
        {
            coefficients[j] = rng() % kMod;
            limit = (limit + std::uint64_t{coefficients[j]} * reference_limit(ratio, j)) % kMod;
        }
        const auto evaluate = [&](std::uint64_t x)
        {
            std::uint64_t value = 0;
            for (auto it = coefficients.rbegin(); it != coefficients.rend(); ++it)
            {
                value = (value * x + *it) % kMod;
            }
            return value;
        };
        for (std::size_t j = 0; j < samples.size(); ++j)
        {
            samples[j] = static_cast<std::uint32_t>(evaluate(j));
        }
        std::uint64_t expected = 0, weight = 1;
        for (std::uint64_t j = 0; j < length; ++j)
        {
            expected = (expected + weight * evaluate(j)) % kMod;
            weight = weight * ratio % kMod;
        }
        assert(sum_exponential_polynomial(samples, ratio, length) == expected);
        assert(sum_exponential_polynomial_limit(samples, ratio) == limit);
    }
    try
    {
        static_cast<void>(sum_exponential_powers(2, kMod - 1, 5));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    try
    {
        sirkit::detail::check_exponential_sample_count(kMod);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
}
