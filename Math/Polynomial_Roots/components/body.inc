// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_POLYNOMIAL_ROOTS_DEFINED
#define SIRKIT_POLYNOMIAL_ROOTS_DEFINED
namespace reinsirk::detail
{
inline FormalPowerSeries RootFinderPower(FormalPowerSeries base, std::uint32_t exponent, const FormalPowerSeries &modulus)
{
    FormalPowerSeries result{1};
    while (exponent != 0)
    {
        if ((exponent & 1U) != 0)
        {
            result = (result * base) % modulus;
        }
        exponent >>= 1;
        if (exponent != 0)
        {
            base = (base * base) % modulus;
        }
    }
    return result;
}

inline std::uint32_t LinearPolynomialRoot(const FormalPowerSeries &value)
{
    constexpr auto kMod = FormalPowerSeries::modulus();
    return fps_multiply(kMod - value[0], fps_power(value[1], kMod - 2));
}
}

inline std::vector<std::uint32_t> polynomial_roots(FormalPowerSeries value, std::uint64_t seed = std::random_device{}())
{
    constexpr auto kMod = FormalPowerSeries::modulus();
    value.trim();
    if (value.empty())
    {
        throw std::domain_error("polynomial_roots: every residue is a root of zero");
    }
    if (value.size() > (std::size_t{1} << 21))
    {
        throw std::length_error("polynomial_roots: degree exceeds NTT budget");
    }
    if (value.size() == 1)
    {
        return {};
    }
    if (value.size() == 2)
    {
        return {::reinsirk::detail::LinearPolynomialRoot(value)};
    }

    // x^p-xは法pで全ての一次因子を1回ずつ含む。
    const FormalPowerSeries x{0, 1};
    auto linear_factors = polynomial_extended_gcd(value, ::reinsirk::detail::RootFinderPower(x, kMod, value) - x).gcd;
    if (linear_factors.size() == 1)
    {
        return {};
    }
    std::mt19937_64 engine(seed);
    std::uniform_int_distribution<std::uint32_t> residue(0, kMod - 1);
    std::vector<std::uint32_t> roots;
    roots.reserve(linear_factors.size() - 1);
    std::vector<FormalPowerSeries> pending;
    pending.push_back(std::move(linear_factors));
    while (!pending.empty())
    {
        auto factor = std::move(pending.back());
        pending.pop_back();
        if (factor.size() == 2)
        {
            roots.push_back(::reinsirk::detail::LinearPolynomialRoot(factor));
            continue;
        }
        while (true)
        {
            // 残った根についてx+shiftが非零平方剰余となる因子を取り出す。
            auto quadratic = ::reinsirk::detail::RootFinderPower(FormalPowerSeries{residue(engine), 1}, (kMod - 1) / 2, factor);
            auto split = polynomial_extended_gcd(factor, std::move(quadratic) - FormalPowerSeries{1}).gcd;
            if (split.size() > 1 && split.size() < factor.size())
            {
                pending.push_back(factor / split);
                pending.push_back(std::move(split));
                break;
            }
        }
    }
    std::ranges::sort(roots);
    return roots;
}
#endif
