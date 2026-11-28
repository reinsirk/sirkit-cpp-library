// SPDX-License-Identifier: CC0-1.0
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_STIRLING_COLUMNS_DEFINED
#define SIRKIT_STIRLING_COLUMNS_DEFINED
namespace reinsirk::detail
{
template <bool First>
std::vector<std::uint32_t> StirlingColumn(std::size_t n, std::size_t k)
{
    if (k > n)
    {
        return {};
    }
    if (n >= fps_modulus || n - k >= FormalPowerSeries::max_precision())
    {
        throw std::length_error("stirling_column: index exceeds modulus or precision");
    }
    const auto length = n - k + 1;
    std::vector<std::uint32_t> coefficients(length);
    if (k == 0)
    {
        coefficients[0] = 1;
        return coefficients;
    }
    fps_prepare_inverses(length);
    const auto &inverses = fps_integer_inverses();
    // 指数型母関数 log(1+x)^k/k! と (exp(x)-1)^k/k! から x^k を除いて計算する。
    if constexpr (First)
    {
        for (std::size_t i = 0; i < length; ++i)
        {
            coefficients[i] = i % 2 == 0 ? inverses[i + 1] : fps_modulus - inverses[i + 1];
        }
    }
    else
    {
        std::uint32_t inverse_factorial = 1;
        for (std::size_t i = 0; i < length; ++i)
        {
            inverse_factorial = fps_multiply(inverse_factorial, inverses[i + 1]);
            coefficients[i] = inverse_factorial;
        }
    }
    const auto powered = FormalPowerSeries(std::move(coefficients)).pow(k, length);
    std::vector<std::uint32_t> result(length);
    std::uint32_t factorial_ratio = 1;
    for (std::size_t i = 0; i < length; ++i)
    {
        result[i] = fps_multiply(powered[i], factorial_ratio);
        factorial_ratio = fps_multiply(factorial_ratio, static_cast<std::uint32_t>(k + i + 1));
    }
    return result;
}
}

inline std::vector<std::uint32_t> stirling_first_column(std::size_t n, std::size_t k)
{
    return ::reinsirk::detail::StirlingColumn<true>(n, k);
}

inline std::vector<std::uint32_t> stirling_second_column(std::size_t n, std::size_t k)
{
    return ::reinsirk::detail::StirlingColumn<false>(n, k);
}
#endif
