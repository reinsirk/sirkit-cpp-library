// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Polynomial_GCD.cpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Polynomial = sirkit::FormalPowerSeries;
using Coefficients = std::vector<std::uint32_t>;
constexpr auto kMod = Polynomial::modulus();

std::uint32_t Mul(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kMod);
}

std::uint32_t Reciprocal(std::uint32_t value)
{
    std::uint32_t result = 1u;
    for (auto exponent = kMod - 2; exponent > 0; exponent >>= 1)
    {
        if (exponent % 2 != 0)
        {
            result = Mul(result, value);
        }
        value = Mul(value, value);
    }
    return result;
}

void Trim(Coefficients &value)
{
    while (!value.empty() && value.back() == 0)
    {
        value.pop_back();
    }
}

Coefficients EuclideanGcd(Coefficients a, Coefficients b)
{
    Trim(a);
    Trim(b);
    while (!b.empty())
    {
        const auto inverse = Reciprocal(b.back());
        for (auto end = a.size(); end >= b.size(); --end)
        {
            const auto quotient = Mul(a[end - 1], inverse);
            for (std::size_t j = 0, i = end - b.size(); j < b.size(); ++j)
            {
                a[i + j] = (a[i + j] + kMod - Mul(quotient, b[j])) % kMod;
            }
        }
        Trim(a);
        a.swap(b);
    }
    if (!a.empty())
    {
        const auto inverse = Reciprocal(a.back());
        for (auto &coefficient : a)
        {
            coefficient = Mul(coefficient, inverse);
        }
    }
    return a;
}

Polynomial Random(std::size_t n, std::mt19937 &random)
{
    Coefficients coefficients(n);
    for (auto &coefficient : coefficients)
    {
        coefficient = static_cast<std::uint32_t>(random() % kMod);
    }
    return Polynomial(std::move(coefficients));
}

void Compare(const Polynomial &a, const Polynomial &b)
{
    const auto result = sirkit::polynomial_extended_gcd(a, b);
    const auto expected = EuclideanGcd(a.coefficients(), b.coefficients());
    assert(result.gcd.coefficients() == expected);
    auto recovered = result.first * a + result.second * b;
    recovered.trim();
    assert(recovered == result.gcd);
    if (!b.empty())
    {
        const auto inverse = sirkit::polynomial_inverse_mod(a, b);
        assert(inverse.has_value() == (expected.size() == 1));
        if (inverse)
        {
            assert(inverse->size() < b.size());
            assert((a * *inverse) % b == Polynomial{1} % b);
        }
    }
}
}

int main()
{
    Compare({}, {});
    Compare({2, 0, 1}, {});
    Compare({}, {2});
    Compare({}, {1, 2});
    Compare({2, 0, 1}, {0, 1});
    Compare({0, 1, 2}, {0, 1});
    Compare({3, 0, 0, 2, 0, 0}, {2, 1, 0});
    try
    {
        static_cast<void>(sirkit::polynomial_inverse_mod({1}, {0, 0}));
        assert(false);
    }
    catch (const std::domain_error &)
    {
    }
    try
    {
        Polynomial oversized((std::size_t{1} << 21) + 1);
        oversized.set(oversized.size() - 1, 1);
        static_cast<void>(sirkit::polynomial_extended_gcd(std::move(oversized), {1}));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }

    std::mt19937 random(45721);
    for (const std::size_t size : {32, 63, 64, 65, 66, 96, 129, 250, 369})
    {
        const auto a = Random(size, random);
        for (const auto width : {size / 2, size / 2 + 1, size - 1, size, size + 1})
        {
            const auto b = Random(width, random);
            Compare(a, b);
            const auto common = Random(5, random);
            Compare(a * common, b * common);
        }
    }
    const auto factor = Random(65, random);
    const auto left = Polynomial{1, 0, 0, 7} * factor;
    const auto right = Polynomial{4, 0, 1} * factor;
    Compare(left, right);
    Compare(left, left);
    Compare(Polynomial{3, 2} * factor, factor);
}
