// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

namespace
{
using Polynomial = sirkit::FormalPowerSeries;
constexpr auto kMod = Polynomial::modulus();

std::uint32_t Multiply(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kMod);
}

std::uint32_t Power(std::uint32_t a, std::size_t exponent)
{
    std::uint32_t result = 1;
    for (; exponent != 0; exponent >>= 1)
    {
        if (exponent % 2 != 0)
        {
            result = Multiply(result, a);
        }
        a = Multiply(a, a);
    }
    return result;
}

struct Fraction
{
    Polynomial numerator;
    Polynomial denominator;
};

Fraction Merge(std::span<const std::pair<std::uint32_t, std::uint32_t>> terms)
{
    if (terms.empty())
    {
        return {{}, {1}};
    }
    if (terms.size() == 1)
    {
        const auto [value, ratio] = terms.front();
        return {{value}, {1, -static_cast<std::int64_t>(ratio)}};
    }
    auto left = Merge(terms.first(terms.size() / 2));
    auto right = Merge(terms.subspan(terms.size() / 2));
    return {left.numerator * right.denominator + right.numerator * left.denominator,
            left.denominator * right.denominator};
}
}

int main()
{
    std::size_t n{}, faces{}, players{};
    verify_in >> n >> faces >> players;
    const auto probability = Power(static_cast<std::uint32_t>(faces), kMod - 2);
    Polynomial dice(n);
    for (std::size_t i = 0; i < faces; ++i)
    {
        std::size_t advance{};
        verify_in >> advance;
        if (advance < n)
        {
            dice.set(advance, probability);
        }
    }
    const Polynomial prefix(std::vector<std::uint32_t>(n, 1));
    // 生存確率F(k)=[x^(N-1)] (1+x+...+x^(N-1))*dice(x)^k。
    const auto survival = sirkit::fps_power_projection(dice, prefix, n - 1, n);
    std::vector<std::pair<std::uint32_t, std::uint32_t>> terms;
    std::uint32_t last = 0;
    for (std::size_t k = 1; k <= n; ++k)
    {
        const auto before = survival[k - 1];
        const auto after = survival.get(k);
        const auto goal = (before + kMod - after) % kMod;
        if (goal == 0)
        {
            continue;
        }
        last = (last + Multiply(goal, Power(after, players - 1))) % kMod;
        if (before != 0)
        {
            const auto ratio = Multiply(after, Power(before, kMod - 2));
            terms.emplace_back(Multiply(goal, Power(before, players - 1)), ratio);
        }
    }
    const auto merged = Merge(terms);
    const auto wins = merged.numerator * merged.denominator.inverse(players - 1);
    for (std::size_t i = 0; i + 1 < players; ++i)
    {
        verify_out << wins.get(i) << '\n';
    }
    verify_out << last << '\n';
}
