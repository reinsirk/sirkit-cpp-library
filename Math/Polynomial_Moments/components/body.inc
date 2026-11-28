#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_POLYNOMIAL_MOMENTS_DEFINED
#define SIRKIT_POLYNOMIAL_MOMENTS_DEFINED

namespace detail
{
constexpr std::size_t kMomentLeafSize = 16;

struct MomentFraction
{
    FormalPowerSeries numerator;
    FormalPowerSeries denominator;
};

inline MomentFraction BuildMomentFraction(std::span<const std::uint32_t> points,
                                          std::span<const std::uint32_t> weights, std::size_t count)
{
    if (points.size() <= kMomentLeafSize)
    {
        const auto size = std::min(count, points.size() + 1);
        std::vector<std::uint32_t> numerator(size), denominator(size);
        denominator[0] = 1;
        for (std::size_t i = 0; i < points.size(); ++i)
        {
            for (auto j = std::min(size - 1, i + 1); j != 0; --j)
            {
                numerator[j] = fps_add(fps_subtract(numerator[j], fps_multiply(points[i], numerator[j - 1])),
                                       fps_multiply(weights[i], denominator[j]));
                denominator[j] = fps_subtract(denominator[j], fps_multiply(points[i], denominator[j - 1]));
            }
            numerator[0] = fps_add(numerator[0], weights[i]);
        }
        numerator.resize(std::min(points.size(), count));
        return {FormalPowerSeries(std::move(numerator)), FormalPowerSeries(std::move(denominator))};
    }

    const auto middle = points.size() / 2;
    const auto left = BuildMomentFraction(points.first(middle), weights.first(middle), count);
    const auto right = BuildMomentFraction(points.subspan(middle), weights.subspan(middle), count);
    auto numerator = left.numerator * right.denominator + right.numerator * left.denominator;
    auto denominator = left.denominator * right.denominator;
    numerator.resize(std::min(numerator.size(), count));
    denominator.resize(std::min(denominator.size(), count));
    return {std::move(numerator), std::move(denominator)};
}

inline FormalPowerSeries WeightedMomentsNormalized(std::span<const std::uint32_t> points,
                                                   std::span<const std::uint32_t> weights, std::size_t count)
{
    if (std::min(points.size(), count) <= kMomentLeafSize)
    {
        std::vector<std::uint32_t> moments(count);
        for (std::size_t i = 0; i < points.size(); ++i)
        {
            auto term = weights[i];
            for (auto &moment : moments)
            {
                moment = fps_add(moment, term);
                term = fps_multiply(term, points[i]);
            }
        }
        return FormalPowerSeries(std::move(moments));
    }
    // Σ w/(1-p*x) を分数のまま足し、最後に一度だけ分母の逆数を取る。
    const auto fraction = BuildMomentFraction(points, weights, count);
    auto moments = fraction.numerator * fraction.denominator.inverse(count);
    moments.resize(count);
    return moments;
}
} // namespace detail

template <std::integral Point, std::integral Weight>
[[nodiscard]] FormalPowerSeries weighted_power_sums(const std::vector<Point> &points, const std::vector<Weight> &weights,
                                                    std::size_t count)
{
    if (points.size() != weights.size())
    {
        throw std::invalid_argument("weighted_power_sums: expected one weight per point");
    }
    detail::fps_require_precision(count);
    if (count == 0)
    {
        return {};
    }
    std::vector<std::uint32_t> residues(points.size()), coefficients(weights.size());
    for (std::size_t i = 0; i < points.size(); ++i)
    {
        residues[i] = detail::polynomial_normalize(points[i]);
        coefficients[i] = detail::polynomial_normalize(weights[i]);
    }
    return detail::WeightedMomentsNormalized(residues, coefficients, count);
}

[[nodiscard]] inline FormalPowerSeries exponential_composition(const FormalPowerSeries &polynomial, std::int64_t scale,
                                                               std::size_t count, std::int64_t shift = 0)
{
    detail::fps_require_precision(count);
    detail::fps_require_precision(polynomial.size());
    if (count == 0 || polynomial.empty())
    {
        return FormalPowerSeries(count);
    }
    const auto step = detail::polynomial_normalize(scale);
    const auto offset = detail::polynomial_normalize(shift);
    if (step == 0)
    {
        FormalPowerSeries result(count);
        result.set(0, detail::polynomial_horner(polynomial, detail::fps_add(1, offset)));
        return result;
    }
    const auto shifted = offset == 0 ? polynomial : TaylorShift(polynomial, offset);
    std::vector<std::uint32_t> powers(shifted.size());
    std::iota(powers.begin(), powers.end(), 0u);
    auto result = detail::WeightedMomentsNormalized(powers, shifted.coefficients(), count);
    detail::fps_prepare_inverses(count - 1);
    const auto &inverses = detail::fps_integer_inverses();
    std::uint32_t factor = 1;
    for (std::size_t i = 1; i < count; ++i)
    {
        factor = detail::fps_multiply(detail::fps_multiply(factor, step), inverses[i]);
        result.set(i, detail::fps_multiply(result.get(i), factor));
    }
    return result;
}

#endif
