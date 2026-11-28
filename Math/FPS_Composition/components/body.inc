// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_FPS_COMPOSITION_DEFINED
#define SIRKIT_FPS_COMPOSITION_DEFINED
namespace reinsirk::detail
{
inline constexpr auto kCompositionCapacity = std::size_t{1} << 18;

inline void CheckCompositionPrecision(std::size_t precision)
{
    if (precision > kCompositionCapacity)
    {
        throw std::length_error("FPS composition: precision exceeds NTT budget");
    }
}

struct CompositionRectangle
{
    std::size_t x_width;
    std::size_t y_width;
    std::vector<std::uint32_t> values;

    CompositionRectangle(std::size_t x_size, std::size_t y_size)
        : x_width(x_size), y_width(y_size), values(x_size * y_size)
    {
    }

    std::uint32_t &at(std::size_t x, std::size_t y)
    {
        return values[y * x_width + x];
    }

    std::uint32_t at(std::size_t x, std::size_t y) const
    {
        return values[y * x_width + x];
    }
};

inline CompositionRectangle CompositionGraeffe(const CompositionRectangle &denominator, std::size_t y_limit)
{
    const auto half_width = (denominator.x_width + 1) / 2;
    const auto odd_width = denominator.x_width / 2;
    const auto stride = 2 * half_width;
    const auto y_last = denominator.y_width - 1;
    std::vector<std::uint32_t> even(y_last * stride + half_width), odd(y_last * stride + odd_width);
    for (std::size_t y = 0; y <= y_last; ++y)
    {
        const auto offset = y * stride;
        for (std::size_t x = 0; x < half_width; ++x)
        {
            even[offset + x] = denominator.at(2 * x, y);
        }
        for (std::size_t x = 0; x < odd_width; ++x)
        {
            odd[offset + x] = denominator.at(2 * x + 1, y);
        }
    }
    // Q(x,y)=E(x²,y)+xO(x²,y)なら、Q(x,y)Q(-x,y)=E(x²,y)^2-x²O(x²,y)^2。
    const auto even_squared = fps_engine().square(even);
    const auto odd_squared = fps_engine().square(odd);
    CompositionRectangle result(half_width, std::min(y_limit, 2 * y_last + 1));
    for (std::size_t y = 0; y < result.y_width; ++y)
    {
        const auto offset = y * stride;
        result.at(0, y) = even_squared[offset];
        for (std::size_t x = 1; x < half_width; ++x)
        {
            result.at(x, y) = fps_subtract(even_squared[offset + x], odd_squared[offset + x - 1]);
        }
    }
    return result;
}

inline std::vector<std::uint32_t> CompositionProduct(const CompositionRectangle &numerator,
                                                     const CompositionRectangle &denominator,
                                                     std::size_t stride, std::size_t stretch)
{
    std::vector<std::uint32_t> left((numerator.y_width - 1) * stride + stretch * (numerator.x_width - 1) + 1);
    std::vector<std::uint32_t> right((denominator.y_width - 1) * stride + denominator.x_width);
    for (std::size_t y = 0; y < numerator.y_width; ++y)
    {
        for (std::size_t x = 0; x < numerator.x_width; ++x)
        {
            left[y * stride + stretch * x] = numerator.at(x, y);
        }
    }
    for (std::size_t y = 0; y < denominator.y_width; ++y)
    {
        for (std::size_t x = 0; x < denominator.x_width; ++x)
        {
            const auto coefficient = denominator.at(x, y);
            right[y * stride + x] = (x % 2 == 0) ? coefficient : fps_subtract(0, coefficient);
        }
    }
    return fps_engine().convolution_normalized(std::move(left), std::move(right));
}

inline CompositionRectangle CompositionDenominator(const FormalPowerSeries &series, std::size_t precision)
{
    CompositionRectangle denominator(precision, 2);
    denominator.at(0, 0) = 1;
    for (std::size_t x = 0; x < precision; ++x)
    {
        denominator.at(x, 1) = fps_subtract(0, series.get(x));
    }
    return denominator;
}

class FpsComposer
{
    const FormalPowerSeries &outer_;
    const std::size_t outer_size_;

    [[nodiscard]] CompositionRectangle restore(const CompositionRectangle &denominator,
                                               const CompositionRectangle &reduced,
                                               std::size_t y_skip, std::size_t y_count) const
    {
        const auto stride = 2 * denominator.x_width - 1;
        const auto product = CompositionProduct(reduced, denominator, stride, 2);
        CompositionRectangle result(denominator.x_width, y_count);
        for (std::size_t y = 0; y < y_count; ++y)
        {
            std::copy_n(product.data() + (y_skip + y) * stride, result.x_width,
                        result.values.data() + y * result.x_width);
        }
        return result;
    }

    [[nodiscard]] CompositionRectangle extract(const CompositionRectangle &denominator, std::size_t start) const
    {
        const auto y_count = outer_size_ - start;
        if (denominator.x_width == 1)
        {
            CompositionRectangle answer(1, y_count); // g(0)=0なら各段階でQ(0,y)=1。
            for (std::size_t y = 0; y < y_count; ++y)
            {
                answer.at(0, y) = outer_.get(outer_size_ - 1 - start - y);
            }
            return answer;
        }
        const auto maximum_degree = denominator.y_width - 1; // y^start以上に寄与し得る係数だけを残す。
        const auto reduced_start = start - std::min(start, maximum_degree);
        const auto window = extract(CompositionGraeffe(denominator, outer_size_), reduced_start);
        return restore(denominator, window, start - reduced_start, y_count);
    }

public:
    FpsComposer(const FormalPowerSeries &outer, std::size_t outer_size) : outer_(outer), outer_size_(outer_size)
    {
    }

    [[nodiscard]] FormalPowerSeries run(const FormalPowerSeries &inner, std::size_t precision) const
    {
        return FormalPowerSeries(extract(CompositionDenominator(inner, precision), outer_size_ - 1).values); // [y^(m-1)] (y^(m-1)f(1/y))/(1-yg(x))。
    }
};
inline FormalPowerSeries ProjectFpsPowers(const FormalPowerSeries &value, const FormalPowerSeries &weight,
                                          std::size_t degree, std::size_t count)
{
    if (count == 0)
    {
        return {};
    }
    if (degree >= kCompositionCapacity || count > kCompositionCapacity)
    {
        throw std::length_error("fps_power_projection: degree or count exceeds NTT budget");
    }
    if (count == 1)
    {
        return FormalPowerSeries{weight.get(degree)};
    }
    auto denominator = CompositionDenominator(value, degree + 1); // [x^degree]weight(x)/(1-y*value(x))を取り出す。
    CompositionRectangle numerator(degree + 1, 1);
    for (std::size_t x = 0; x <= degree; ++x)
    {
        numerator.at(x, 0) = weight.get(x);
    }
    for (; degree > 0; degree /= 2)
    {
        const auto stride = 2 * denominator.x_width - 1;
        const auto width = std::min(count, numerator.y_width + denominator.y_width - 1);
        const auto product = CompositionProduct(numerator, denominator, stride, 1);
        CompositionRectangle selected(degree / 2 + 1, width);
        for (std::size_t y = 0; y < width; ++y)
        {
            for (std::size_t x = 0; x < selected.x_width; ++x)
            {
                selected.at(x, y) = product[y * stride + 2 * x + degree % 2];
            }
        }
        numerator = std::move(selected);
        denominator = CompositionGraeffe(denominator, count);
    }
    const FormalPowerSeries upper(std::move(numerator.values));
    FormalPowerSeries lower(std::move(denominator.values));
    lower.trim();
    if (lower.size() == 1)
    {
        return upper.prefix(count);
    }
    return (upper * lower.inverse(count)).prefix(count);
}

}

inline FormalPowerSeries fps_compose(const FormalPowerSeries &outer, const FormalPowerSeries &inner,
                                     std::size_t precision)
{
    if (precision == 0)
    {
        return {};
    }
    ::reinsirk::detail::CheckCompositionPrecision(precision);
    if (inner.get(0) != 0)
    {
        throw std::invalid_argument("fps_compose: inner constant must be zero");
    }
    const auto inner_size = std::min(inner.size(), precision);
    std::size_t leading = 1;
    while (leading < inner_size && inner[leading] == 0)
    {
        ++leading;
    }
    const auto outer_size = std::min(outer.size(), 1 + (precision - 1) / leading);
    if (leading >= inner_size || outer_size <= 1)
    {
        FormalPowerSeries answer(precision);
        answer.set(0, outer.get(0));
        return answer;
    }
    return ::reinsirk::detail::FpsComposer(outer, outer_size).run(inner, precision);
}

inline FormalPowerSeries fps_compositional_inverse(const FormalPowerSeries &value, std::size_t precision)
{
    if (precision == 0)
    {
        return {};
    }
    ::reinsirk::detail::CheckCompositionPrecision(precision);
    if (value.get(0) != 0 || value.get(1) == 0)
    {
        throw std::invalid_argument("fps_compositional_inverse: expected zero constant and nonzero linear term");
    }
    if (precision == 1)
    {
        return FormalPowerSeries(precision);
    }
    constexpr auto kMod = FormalPowerSeries::modulus();
    FormalPowerSeries result{0, ::reinsirk::detail::fps_power(value.get(1), kMod - 2)};
    for (std::size_t width = 2; width < precision;)
    {
        const auto target = std::min(2 * width, precision);
        const auto tail = target - width;
        const auto composed = fps_compose(value, result, target);
        std::vector<std::uint32_t> error(tail);
        std::copy_n(composed.coefficients().data() + width, tail, error.begin());
        const auto reciprocal = composed.derivative().inverse(tail); // f'(g)=(f(g))'/g'から上側のNewton補正を計算。
        const auto correction = (FormalPowerSeries(std::move(error)) * result.derivative().prefix(tail)).prefix(tail) * reciprocal;
        result.resize(target);
        for (std::size_t i = 0; i < tail; ++i)
        {
            result.set(width + i, -static_cast<std::int64_t>(correction[i]));
        }
        width = target;
    }
    return result;
}
inline FormalPowerSeries fps_power_projection(const FormalPowerSeries &value, const FormalPowerSeries &weight,
                                              std::size_t degree, std::size_t count)
{
    return ::reinsirk::detail::ProjectFpsPowers(value, weight, degree, count);
}
#endif
