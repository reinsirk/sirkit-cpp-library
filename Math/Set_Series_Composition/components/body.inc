// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_SET_SERIES_COMPOSITION_DEFINED
#define SIRKIT_SET_SERIES_COMPOSITION_DEFINED
namespace detail
{
inline std::uint32_t SetCompositionAdd(std::uint32_t a, std::uint32_t b) noexcept
{
    const auto sum = a + b;
    return sum < kSetSeriesMod ? sum : sum - kSetSeriesMod;
}

inline std::uint32_t SetCompositionMul(std::uint32_t a, std::uint32_t b) noexcept
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kSetSeriesMod);
}

inline std::vector<std::uint32_t> SetCompose(std::span<const std::uint32_t> polynomial,
                                             std::span<const std::uint32_t> series)
{
    CheckSetSeriesSize(series.size());
    if (polynomial.empty())
    {
        return std::vector<std::uint32_t>(series.size());
    }
    const auto constant = series.front() % kSetSeriesMod;
    const auto variables = static_cast<std::size_t>(std::countr_zero(series.size()));
    const auto degrees = std::min(variables + 1, polynomial.size());
    std::vector<std::vector<std::uint32_t>> values(degrees, {0}); // Horner法でfと導関数を定数項で評価。
    for (auto term = polynomial.rbegin(); term != polynomial.rend(); ++term)
    {
        for (std::size_t i = degrees - 1; i > 0; --i)
        {
            values[i][0] = SetCompositionAdd(SetCompositionMul(values[i][0], constant),
                                             SetCompositionMul(static_cast<std::uint32_t>(i), values[i - 1][0]));
        }
        values.front().front() = SetCompositionAdd(SetCompositionMul(values.front().front(), constant),
                                                   *term % kSetSeriesMod);
    }
    for (std::size_t length = 1; length < series.size(); length *= 2)
    {
        const auto upper = series.subspan(length, length); // x^2=0ならf(a+xb) = f(a) + xb f'(a)。
        const auto left = static_cast<std::size_t>(std::countr_zero(series.size() / length));
        const auto next_count = std::min(left, degrees);
        for (std::size_t i = 0; i < next_count; ++i)
        {
            if (i + 1 < values.size())
            {
                auto high = SetSubsetProduct(values[i + 1], upper);
                values[i].insert(values[i].end(), high.begin(), high.end());
            }
            else
            {
                values[i].resize(2 * length);
            }
        }
        values.resize(next_count);
    }
    return std::move(values.front());
}

inline std::vector<std::uint32_t> SetProject(std::span<const std::uint32_t> series,
                                             std::span<const std::uint32_t> weights, std::size_t count)
{
    CheckSetSeriesSize(series.size());
    if (series.size() != weights.size())
    {
        throw std::invalid_argument("set_series_power_projection: size mismatch");
    }
    if (count == 0)
    {
        return {};
    }
    std::vector<std::vector<std::uint32_t>> dual(1, std::vector<std::uint32_t>(weights.size()));
    for (std::size_t i = 0; i < weights.size(); ++i)
    {
        dual.front()[i] = weights[i] % kSetSeriesMod;
    }
    for (std::size_t length = series.size() / 2; length != 0; length /= 2)
    {
        const auto limit = std::min(dual.size() + 1, count);
        std::vector<std::vector<std::uint32_t>> reduced(limit, std::vector<std::uint32_t>(length));
        const auto upper = series.subspan(length, length);
        for (std::size_t i = 0; i < dual.size(); ++i)
        {
            for (std::size_t j = 0; j < length; ++j)
            {
                reduced[i][j] = SetCompositionAdd(reduced[i][j], dual[i][j]);
            }
            if (i + 1 < limit)
            {
                std::vector<std::uint32_t> reversed(dual[i].rbegin(), dual[i].rbegin() + length); // 補集合で添字を反転して転置。
                const auto cross = SetSubsetProduct(reversed, upper);
                std::ranges::reverse_copy(cross, reduced[i + 1].begin());
            }
        }
        dual = std::move(reduced);
    }
    const auto constant = series.front() % kSetSeriesMod;
    std::vector<std::uint32_t> powers(count, 1), result(count);
    for (std::size_t i = 1; i < count; ++i)
    {
        powers[i] = SetCompositionMul(powers[i - 1], constant);
    }
    for (std::size_t exponent = 0; exponent < count; ++exponent)
    {
        std::uint32_t falling = 1; // (d/dx)^i x^exponentを定数項で評価し、導関数の計算を転置。
        auto &value = result[exponent];
        for (std::size_t i = 0; i <= std::min(exponent, dual.size() - 1); ++i)
        {
            value = SetCompositionAdd(value, SetCompositionMul(dual[i][0],
                                                               SetCompositionMul(falling, powers[exponent - i])));
            falling = SetCompositionMul(falling, static_cast<std::uint32_t>((exponent - i) % kSetSeriesMod));
        }
    }
    return result;
}
}

inline std::vector<std::uint32_t> set_series_compose(std::span<const std::uint32_t> polynomial,
                                                     std::span<const std::uint32_t> series)
{
    return detail::SetCompose(polynomial, series);
}

inline std::vector<std::uint32_t> set_series_power_projection(std::span<const std::uint32_t> series,
                                                              std::span<const std::uint32_t> weights,
                                                              std::size_t count)
{
    return detail::SetProject(series, weights, count);
}
#endif
