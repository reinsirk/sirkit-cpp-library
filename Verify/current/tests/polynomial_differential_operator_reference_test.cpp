// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Polynomial_Differential_Operator.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
constexpr auto kMod = std::uint64_t{998244353};
using sirkit::FormalPowerSeries;

FormalPowerSeries direct(const FormalPowerSeries &initial, const FormalPowerSeries &p, const FormalPowerSeries &a,
                         std::size_t steps, std::size_t precision)
{
    auto values = initial.prefix(steps + precision).coefficients();
    for (std::size_t t = 0; t < steps; ++t)
    {
        std::vector<std::uint32_t> next(values.size() - 1);
        for (std::size_t i = 0; i < next.size(); ++i)
        {
            std::uint64_t sum = 0;
            for (std::size_t j = 0; j <= i; ++j)
            {
                sum += std::uint64_t{a.get(j)} * values[i - j] % kMod;
                sum += std::uint64_t{p.get(j)} * values[i - j + 1] % kMod * (i - j + 1) % kMod;
            }
            next[i] = static_cast<std::uint32_t>(sum % kMod);
        }
        values = std::move(next);
    }
    return FormalPowerSeries(std::move(values));
}
} // namespace

int main()
{
    using sirkit::polynomial_differential_operator_power;
    assert(polynomial_differential_operator_power({1}, {}, {}, 8, 0).empty());
    assert(polynomial_differential_operator_power({1, 2, 3}, {}, {}, 0, 2) == FormalPowerSeries({1, 2}));
    assert(polynomial_differential_operator_power({0, 1}, {-1, 0, 1}, {}, 3, 5) ==
           FormalPowerSeries({2, 0, -8, 0, 6}));
    assert(polynomial_differential_operator_power({0, 1}, {3}, {}, 1, 1) == FormalPowerSeries({3}));
    assert(polynomial_differential_operator_power({}, {1, 2, 3}, {-1, 4}, 8, 4) == FormalPowerSeries(4));
    bool rejected = false;
    try
    {
        static_cast<void>(polynomial_differential_operator_power({}, {0, 1}, {}, 1, 2));
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected);

    constexpr auto kBudget = std::size_t{1} << 18;
    assert(polynomial_differential_operator_power({1}, {}, {}, 0, kBudget).size() == kBudget);
    for (const auto &[steps, precision] : {std::pair{0U, kBudget + 1}, std::pair{1U, kBudget}})
    {
        rejected = false;
        try
        {
            static_cast<void>(polynomial_differential_operator_power({}, {}, {}, steps, precision));
        }
        catch (const std::length_error &)
        {
            rejected = true;
        }
        assert(rejected);
    }

    std::mt19937_64 rng(20261128);
    const auto random_series = [&](std::size_t n)
    {
        std::vector<std::uint32_t> coefficients(n);
        for (auto &coefficient : coefficients)
        {
            coefficient = static_cast<std::uint32_t>(rng() % kMod);
        }
        return FormalPowerSeries(std::move(coefficients));
    };
    for (unsigned round = 0; round < 170; ++round)
    {
        const std::size_t steps = rng() % 22;
        const std::size_t precision = 1 + rng() % 30;
        const auto initial = random_series(rng() % 48);
        auto p = random_series(1 + rng() % 16);
        p.set(0, 1 + rng() % (kMod - 1));
        const auto a = random_series(rng() % 17);
        assert(polynomial_differential_operator_power(initial, p, a, steps, precision) ==
               direct(initial, p, a, steps, precision));
    }
}
