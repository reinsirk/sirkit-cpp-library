// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#ifndef SIRKIT_POLYNOMIAL_GCD_DEFINED
#define SIRKIT_POLYNOMIAL_GCD_DEFINED
struct PolynomialBezout
{
    FormalPowerSeries gcd;
    FormalPowerSeries first;
    FormalPowerSeries second;
};

namespace reinsirk::detail
{
using GcdPolynomial = FormalPowerSeries;

struct GcdTransformation
{
    GcdPolynomial top_left{1}, top_right{}, bottom_left{}, bottom_right{1};
};

inline GcdPolynomial GcdHighPart(const GcdPolynomial &value, std::size_t shift)
{
    if (value.size() <= shift)
    {
        return {};
    }
    const auto &coefficients = value.coefficients();
    return GcdPolynomial(std::vector<std::uint32_t>(coefficients.begin() + shift, coefficients.end()));
}

inline GcdPolynomial GcdCombine(const GcdPolynomial &a, const GcdPolynomial &b,
                                const GcdPolynomial &c, const GcdPolynomial &d)
{
    auto result = a * b + c * d;
    result.trim();
    return result;
}

inline GcdTransformation GcdCompose(const GcdTransformation &left, const GcdTransformation &right)
{
    return {GcdCombine(left.top_left, right.top_left, left.top_right, right.bottom_left),
            GcdCombine(left.top_left, right.top_right, left.top_right, right.bottom_right),
            GcdCombine(left.bottom_left, right.top_left, left.bottom_right, right.bottom_left),
            GcdCombine(left.bottom_left, right.top_right, left.bottom_right, right.bottom_right)};
}

inline std::pair<GcdPolynomial, GcdPolynomial> GcdApply(const GcdTransformation &change, const GcdPolynomial &a, const GcdPolynomial &b)
{
    return {GcdCombine(change.top_left, a, change.top_right, b),
            GcdCombine(change.bottom_left, a, change.bottom_right, b)};
}

inline GcdTransformation GcdEuclideanStep(GcdTransformation previous, const GcdPolynomial &quotient)
{
    auto left = previous.top_left - quotient * previous.bottom_left;
    auto right = previous.top_right - quotient * previous.bottom_right;
    left.trim();
    right.trim();
    return {std::move(previous.bottom_left), std::move(previous.bottom_right), std::move(left), std::move(right)};
}

inline GcdTransformation GcdHalf(const GcdPolynomial &a, const GcdPolynomial &b)
{
    constexpr std::size_t kDirect = 32;
    const auto middle = a.size() / 2;
    if (b.size() <= middle)
    {
        return {};
    }
    if (a.size() <= kDirect)
    {
        auto left = a;
        auto right = b;
        GcdTransformation result;
        while (right.size() > middle)
        {
            auto [quotient, remainder] = left.divmod(right);
            result = GcdEuclideanStep(std::move(result), quotient);
            left = std::move(right);
            right = std::move(remainder);
        }
        return result;
    }

    // 高次側から、右の余りが半分の次数を下回るまでの除算をまとめる。
    auto first = GcdHalf(GcdHighPart(a, middle), GcdHighPart(b, middle));
    auto [left, right] = GcdApply(first, a, b);
    if (right.size() <= middle)
    {
        return first;
    }
    auto [quotient, remainder] = left.divmod(right);
    first = GcdEuclideanStep(std::move(first), quotient);
    const auto shift = 2 * middle + 1 - right.size();
    const auto second = GcdHalf(GcdHighPart(right, shift), GcdHighPart(remainder, shift));
    return GcdCompose(second, first);
}

inline PolynomialBezout ExtendedPolynomialGcd(GcdPolynomial a, GcdPolynomial b)
{
    a.trim();
    b.trim();
    if (std::max(a.size(), b.size()) > (std::size_t{1} << 21))
    {
        throw std::length_error("polynomial_extended_gcd: degree exceeds NTT budget");
    }

    GcdTransformation result;
    if (a.size() < b.size())
    {
        std::swap(a, b);
        std::swap(result.top_left, result.bottom_left);
        std::swap(result.top_right, result.bottom_right);
    }
    while (!b.empty())
    {
        if (a.size() > 64 && a.size() > b.size() && b.size() > a.size() / 2)
        {
            const auto block = GcdHalf(a, b);
            std::tie(a, b) = GcdApply(block, a, b);
            result = GcdCompose(block, result);
            if (b.empty())
            {
                break;
            }
        }
        auto [quotient, remainder] = a.divmod(b);
        result = GcdEuclideanStep(std::move(result), quotient);
        a = std::move(b);
        b = std::move(remainder);
    }
    if (a.empty())
    {
        return {};
    }
    const auto reciprocal = fps_power(a.coefficients().back(), FormalPowerSeries::modulus() - 2);
    return {a.scale(reciprocal), result.top_left.scale(reciprocal), result.top_right.scale(reciprocal)};
}
}

inline PolynomialBezout polynomial_extended_gcd(FormalPowerSeries a, FormalPowerSeries b)
{
    return ::reinsirk::detail::ExtendedPolynomialGcd(std::move(a), std::move(b));
}

inline std::optional<FormalPowerSeries> polynomial_inverse_mod(FormalPowerSeries value, FormalPowerSeries modulus)
{
    modulus.trim();
    if (modulus.empty())
    {
        throw std::domain_error("polynomial_inverse_mod: zero modulus");
    }
    if (modulus.size() == 1)
    {
        return FormalPowerSeries{};
    }
    auto result = polynomial_extended_gcd(std::move(value), std::move(modulus));
    if (result.gcd.size() == 1)
    {
        return std::move(result.first);
    }
    return std::nullopt;
}
#endif
