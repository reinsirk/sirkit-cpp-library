#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_L1_ORIGIN_LINE_FIT_DEFINED
#define SIRKIT_L1_ORIGIN_LINE_FIT_DEFINED

struct L1OriginLineFit
{
    Fraction cost, slope;
    bool swapped;
};

namespace detail
{
using OriginFitPoint = std::pair<std::int32_t, std::int32_t>;

struct OriginFitDirection
{
    std::int64_t numerator, denominator;
};

struct OriginFitCandidate
{
    OriginFitDirection slope;
    FractionWide numerator;
    std::uint64_t quotient, remainder;
};

inline void CheckOriginFitSize([[maybe_unused]] std::size_t size)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("l1_origin_line_fit: too many points");
    }
#endif
}

template <bool Swapped>
OriginFitCandidate FitOriginLineChart(std::span<const OriginFitPoint> points)
{
    std::vector<OriginFitDirection> ratios;
    ratios.reserve(points.size());
    std::int64_t total = 0;
    for (const auto &[x, y] : points)
    {
        const std::int64_t a = Swapped ? y : x;
        const std::int64_t b = Swapped ? x : y;
        if (a == 0)
        {
            continue;
        }
        const std::int64_t weight = a < 0 ? -a : a;
        const std::int64_t numerator = a < 0 ? -b : b;
        ratios.push_back({std::clamp(numerator, -weight, weight), weight});
        total += weight;
    }
    std::sort(ratios.begin(), ratios.end(), [](const auto &a, const auto &b)
              {
                  return a.numerator * b.denominator < b.numerator * a.denominator;
              });
    OriginFitDirection median{0, 1};
    std::int64_t prefix = 0;
    for (const auto &ratio : ratios)
    {
        prefix += ratio.denominator;
        if (prefix >= (total + 1) / 2)
        {
            median = ratio;
            break;
        }
    }

    FractionWide numerator;
    for (const auto &[x, y] : points)
    {
        const auto a = (Swapped ? y : x) * median.numerator;
        const auto b = (Swapped ? x : y) * median.denominator;
        const auto difference = static_cast<std::uint64_t>(std::max(a, b)) -
                                static_cast<std::uint64_t>(std::min(a, b));
        numerator = numerator + FractionWide(difference);
    }
    const auto [whole, remainder] = numerator.divmod(median.denominator);
    // 1点の距離は |x|+|y| 以下。長さが INT_MAX 以下なら商は uint64_t に収まる。
    return {median, numerator, whole.low(), remainder};
}
}

[[nodiscard]] inline L1OriginLineFit l1_origin_line_fit(std::span<const std::pair<std::int32_t, std::int32_t>> points)
{
    detail::CheckOriginFitSize(points.size());
    const auto horizontal = detail::FitOriginLineChart<false>(points);
    const auto vertical = detail::FitOriginLineChart<true>(points);
    const bool use_vertical = vertical.quotient == horizontal.quotient
                                  ? vertical.remainder * horizontal.slope.denominator < horizontal.remainder * vertical.slope.denominator
                                  : vertical.quotient < horizontal.quotient;
    const auto &best = use_vertical ? vertical : horizontal;
    const auto divisor = std::gcd(best.remainder, static_cast<std::uint64_t>(best.slope.denominator));
    const auto reduced = best.numerator.divmod(divisor).first;
    if (!reduced.fits(std::numeric_limits<std::int64_t>::max()))
    {
        throw std::overflow_error("l1_origin_line_fit: reduced cost numerator out of range");
    }
    return {Fraction(static_cast<std::int64_t>(reduced.low()), best.slope.denominator / static_cast<std::int64_t>(divisor)),
            Fraction(best.slope.numerator, best.slope.denominator), use_vertical};
}

#endif
