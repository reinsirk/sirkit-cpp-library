#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_POLYNOMIAL_RECURRENCE_DEFINED
#define SIRKIT_POLYNOMIAL_RECURRENCE_DEFINED

namespace detail
{
inline std::uint32_t PolynomialRecurrenceResidue(std::int64_t value)
{
    const auto residue = value % kConsecutiveMod;
    return static_cast<std::uint32_t>(residue < 0 ? residue + kConsecutiveMod : residue);
}

inline std::vector<std::int64_t> ReverseRecurrenceArgument(std::span<const std::int64_t> polynomial, bool negate)
{
    std::vector<std::int64_t> coefficients;
    coefficients.reserve(polynomial.size());
    for (std::size_t i = 0; i < polynomial.size(); ++i)
    {
        const auto value = polynomial[i] % kConsecutiveMod;
        const bool change_sign = ((i & 1u) != 0) != negate;
        coefficients.push_back(change_sign ? -value : value);
    }
    while (!coefficients.empty() && coefficients.back() == 0)
    {
        coefficients.pop_back();
    }
    return coefficients;
}
} // namespace detail

[[nodiscard]] inline std::uint32_t polynomial_recurrence_term(
    std::span<const std::int64_t> initial, std::span<const std::vector<std::int64_t>> coefficients,
    std::uint64_t index)
{
    const auto order = initial.size();
    if (order == 0 || coefficients.empty() || coefficients.size() - 1 != order)
    {
        throw std::invalid_argument("polynomial_recurrence_term: expected d initial values and d+1 coefficients");
    }
    if (index < order)
    {
        return detail::PolynomialRecurrenceResidue(initial[static_cast<std::size_t>(index)]);
    }
    const auto scale = detail::ReverseRecurrenceArgument(coefficients.back(), false);
    using Polynomial = std::vector<std::int64_t>;
    std::vector<std::vector<Polynomial>> transition(order + 1, std::vector<Polynomial>(order + 1));
    for (std::size_t i = 0; i < order; ++i)
    {
        transition[0][i] = detail::ReverseRecurrenceArgument(coefficients[order - i - 1], true);
    }
    for (std::size_t i = 1; i < order; ++i)
    {
        transition[i][i - 1] = scale;
    }
    transition[order][order] = scale; // 各更新の分母を別の成分でまとめて掛ける。
    const auto last = index - order;
    const auto first = -static_cast<std::int64_t>(last % detail::kConsecutiveMod);
    const auto product = polynomial_matrix_interval_product(transition, first, last + 1);
    const auto denominator = product[order][order];
    if (denominator == 0)
    {
        throw std::domain_error("polynomial_recurrence_term: a leading coefficient evaluated to zero");
    }
    std::uint32_t value = 0;
    for (std::size_t i = 0; i < order; ++i)
    {
        const auto seed = detail::PolynomialRecurrenceResidue(initial[order - i - 1]);
        value += detail::ConsecutiveMultiply(product[0][i], seed);
        if (value >= detail::kConsecutiveMod)
        {
            value -= detail::kConsecutiveMod;
        }
    }
    return detail::ConsecutiveMultiply(value, detail::ConsecutiveInverse(denominator));
}

#endif
