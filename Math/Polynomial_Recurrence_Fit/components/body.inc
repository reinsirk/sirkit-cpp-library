#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_POLYNOMIAL_RECURRENCE_FIT_DEFINED
#define SIRKIT_POLYNOMIAL_RECURRENCE_FIT_DEFINED

namespace detail
{
inline std::size_t PolynomialRecurrenceFitColumns(std::size_t order, std::size_t degree)
{
    if (degree == std::numeric_limits<std::size_t>::max() ||
        degree + 1 > std::vector<std::uint32_t>().max_size() / (order + 1))
    {
        throw std::length_error("fit_polynomial_recurrence: too many coefficients");
    }
    return (order + 1) * (degree + 1);
}
} // namespace detail

[[nodiscard]] inline std::optional<std::vector<std::vector<std::int64_t>>>
fit_polynomial_recurrence(std::span<const std::int64_t> terms, std::size_t order, std::size_t max_degree)
{
    if (order == 0 || order >= terms.size())
    {
        throw std::invalid_argument("fit_polynomial_recurrence: expected order in [1, terms.size())");
    }
    const auto columns = detail::PolynomialRecurrenceFitColumns(order, max_degree);
    const auto width = max_degree + 1;
    std::vector<std::vector<std::uint32_t>> basis(columns);
    std::vector<std::uint32_t> residues;
    residues.reserve(terms.size());
    for (const auto value : terms)
    {
        residues.push_back(detail::PolynomialRecurrenceResidue(value));
    }

    for (std::size_t t = 0; t < terms.size() - order; ++t)
    {
        const auto point = static_cast<std::uint32_t>(t % detail::kConsecutiveMod);
        auto power = std::uint32_t{1};
        std::vector<std::uint32_t> row(columns);
        for (std::size_t degree = 0; degree < width; ++degree)
        {
            for (std::size_t j = 0; j <= order; ++j)
            {
                row[j * width + degree] = detail::ConsecutiveMultiply(power, residues[t + j]);
            }
            power = detail::ConsecutiveMultiply(power, point);
        }
        // 全入力行を保持せず、非零ピボットを1列につき1行ずつ保存。
        for (std::size_t column = 0; column < columns; ++column)
        {
            const auto entry = row[column];
            if (entry == 0)
            {
                continue;
            }
            auto &pivot = basis[column];
            if (pivot.empty())
            {
                const auto inverse = detail::ConsecutiveInverse(entry);
                row[column] = 1;
                for (std::size_t j = column + 1; j < columns; ++j)
                {
                    row[j] = detail::ConsecutiveMultiply(row[j], inverse);
                }
                pivot = std::move(row);
                break;
            }
            row[column] = 0;
            for (std::size_t j = column + 1; j < columns; ++j)
            {
                const auto cancelled = detail::ConsecutiveMultiply(entry, pivot[j]);
                row[j] = row[j] >= cancelled ? row[j] - cancelled : row[j] + (detail::kConsecutiveMod - cancelled);
            }
        }
    }

    auto free_column = order * width;
    while (free_column < columns && !basis[free_column].empty())
    {
        ++free_column;
    }
    if (free_column == columns)
    {
        return std::nullopt;
    }
    // 最後の多項式の自由変数を1とし、他の自由変数を0として後退代入する。
    std::vector<std::uint32_t> coefficients(columns);
    coefficients[free_column] = 1;
    for (std::size_t next = free_column; next != 0; --next)
    {
        const auto column = next - 1;
        const auto &pivot = basis[column];
        if (pivot.empty())
        {
            continue;
        }
        std::uint32_t sum = 0;
        for (std::size_t j = column + 1; j <= free_column; ++j)
        {
            sum += detail::ConsecutiveMultiply(pivot[j], coefficients[j]);
            if (sum >= detail::kConsecutiveMod)
            {
                sum -= detail::kConsecutiveMod;
            }
        }
        coefficients[column] = (detail::kConsecutiveMod - sum) % detail::kConsecutiveMod;
    }

    std::vector<std::vector<std::int64_t>> result(order + 1, std::vector<std::int64_t>(width));
    for (std::size_t j = 0; j <= order; ++j)
    {
        for (std::size_t degree = 0; degree < width; ++degree)
        {
            result[j][degree] = coefficients[j * width + degree];
        }
    }
    return result;
}

#endif
