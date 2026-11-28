#ifndef SIRKIT_EXPONENTIAL_COEFFICIENT_TRANSFORM_DEFINED
#define SIRKIT_EXPONENTIAL_COEFFICIENT_TRANSFORM_DEFINED

namespace detail
{
inline FormalPowerSeries MultiplyTransformFactor(const FormalPowerSeries &series, std::uint32_t step)
{
    std::vector<std::uint32_t> coefficients(series.coefficients());
    for (auto i = coefficients.size(); i > 1; --i)
    {
        coefficients[i - 1] = fps_subtract(coefficients[i - 1], fps_multiply(step, coefficients[i - 2]));
    }
    return FormalPowerSeries(std::move(coefficients));
}
} // namespace detail

[[nodiscard]] inline FormalPowerSeries exponential_coefficient_transform(const FormalPowerSeries &series, std::int64_t step,
                                                                         std::size_t precision)
{
    detail::CheckCompositionPrecision(precision);
    if (precision == 0)
    {
        return {};
    }
    const auto coefficient = FormalPowerSeries{step}.get(0);
    if (coefficient == 0)
    {
        return series.prefix(precision);
    }
    detail::fps_prepare_inverses(precision - 1);
    const auto &inverses = detail::fps_integer_inverses();
    FormalPowerSeries tree(precision), denominator(precision);
    denominator.set(0, 1);
    std::uint32_t inverse_factorial = 1;
    for (std::size_t i = 1; i < precision; ++i)
    {
        inverse_factorial = detail::fps_multiply(inverse_factorial, inverses[i]);
        const auto base = detail::fps_multiply(coefficient, static_cast<std::uint32_t>(i));
        const auto term = detail::fps_multiply(detail::fps_power(base, i - 1), inverse_factorial);
        tree.set(i, term);
        denominator.set(i, -static_cast<std::int64_t>(detail::fps_multiply(coefficient, term)));
    }
    // W=(x*exp(-step*x))^(-1)とし、変数変換の微分因子を除く。
    const auto composed = fps_compose(series, tree, precision);
    return (composed * denominator.inverse(precision)).prefix(precision);
}

[[nodiscard]] inline FormalPowerSeries inverse_exponential_coefficient_transform(const FormalPowerSeries &series, std::int64_t step,
                                                                                 std::size_t precision)
{
    detail::CheckCompositionPrecision(precision);
    if (precision == 0)
    {
        return {};
    }
    const auto coefficient = FormalPowerSeries{step}.get(0);
    if (coefficient == 0)
    {
        return series.prefix(precision);
    }
    detail::fps_prepare_inverses(precision - 1);
    const auto &inverses = detail::fps_integer_inverses();
    FormalPowerSeries inner(precision);
    const auto negative = detail::fps_subtract(0, coefficient);
    std::uint32_t term = 1;
    for (std::size_t i = 1; i < precision; ++i)
    {
        inner.set(i, term);
        term = detail::fps_multiply(detail::fps_multiply(term, negative), inverses[i]);
    }
    return detail::MultiplyTransformFactor(fps_compose(series, inner, precision), coefficient);
}

#endif
