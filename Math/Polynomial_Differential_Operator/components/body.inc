#ifndef SIRKIT_POLYNOMIAL_DIFFERENTIAL_OPERATOR_DEFINED
#define SIRKIT_POLYNOMIAL_DIFFERENTIAL_OPERATOR_DEFINED

namespace detail
{
inline FormalPowerSeries DifferentialOperatorQuadraticFlow(const FormalPowerSeries &derivative, std::size_t precision)
{
    const auto constant = derivative.get(0);
    const auto linear = derivative.get(1);
    const auto negative_quadratic = fps_subtract(0, derivative.get(2));
    fps_prepare_inverses(precision - 1);
    const auto &inverses = fps_integer_inverses();
    std::vector<std::uint32_t> u(precision), v(precision);
    v[0] = 1;
    // u′=linear*u+constant*v, v′=-quadratic*u とすると u/v が X′=P(X) を満たす。
    for (std::size_t i = 1; i < precision; ++i)
    {
        u[i] = fps_multiply(fps_add(fps_multiply(linear, u[i - 1]), fps_multiply(constant, v[i - 1])), inverses[i]);
        v[i] = fps_multiply(fps_multiply(negative_quadratic, u[i - 1]), inverses[i]);
    }
    const auto reciprocal = FormalPowerSeries(std::move(v)).inverse(precision);
    return (FormalPowerSeries(std::move(u)) * reciprocal).prefix(precision);
}

inline FormalPowerSeries DifferentialOperatorDifferentiate(const FormalPowerSeries &series, std::size_t steps,
                                                           std::size_t precision)
{
    fps_prepare_inverses(precision + steps - 1);
    const auto &inverses = fps_integer_inverses();
    std::uint32_t falling = 1;
    for (std::size_t i = 1; i <= steps; ++i)
    {
        falling = fps_multiply(falling, static_cast<std::uint32_t>(i));
    }
    std::vector<std::uint32_t> result(precision);
    for (std::size_t i = 0; i < precision; ++i)
    {
        result[i] = fps_multiply(series.get(i + steps), falling);
        falling = fps_multiply(fps_multiply(falling, static_cast<std::uint32_t>(i + steps + 1)), inverses[i + 1]);
    }
    return FormalPowerSeries(std::move(result));
}
} // namespace detail

[[nodiscard]] inline FormalPowerSeries polynomial_differential_operator_power(const FormalPowerSeries &initial,
                                                                              const FormalPowerSeries &derivative,
                                                                              const FormalPowerSeries &multiplier,
                                                                              std::size_t steps, std::size_t precision)
{
    if (precision > detail::kCompositionCapacity || steps > detail::kCompositionCapacity - precision)
    {
        throw std::length_error("differential operator: steps plus precision exceed composition budget");
    }
    if (precision == 0)
    {
        return {};
    }
    if (steps == 0)
    {
        return initial.prefix(precision);
    }
    if (derivative.get(0) == 0)
    {
        throw std::invalid_argument("differential operator: zero derivative multiplier at x=0");
    }

    const auto known = precision + steps;
    auto derivative_part = derivative.prefix(std::min(derivative.size(), known - 1));
    derivative_part.trim();
    const auto reciprocal = derivative_part.inverse(known - 1);
    const auto coordinate = reciprocal.integral();
    const auto reversed = derivative_part.size() <= 3 ? detail::DifferentialOperatorQuadraticFlow(derivative_part, known)
                                                      : fps_compositional_inverse(coordinate, known);

    auto coefficient = multiplier.prefix(known - 1);
    coefficient.trim();
    const auto scale = (coefficient * reciprocal).prefix(known - 1).integral().exp(known);

    auto initial_part = initial.prefix(known);
    initial_part.trim();
    const auto flat = fps_compose((initial_part * scale).prefix(known), reversed, known);
    // y′(x)=1/P(x) と U′(x)/U(x)=A(x)/P(x) により、遷移はyに関する微分になる。
    const auto applied = detail::DifferentialOperatorDifferentiate(flat, steps, precision);
    return (fps_compose(applied, coordinate, precision) * scale.inverse(precision)).prefix(precision);
}

#endif
