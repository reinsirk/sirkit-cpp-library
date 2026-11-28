#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_TOURNAMENT_STRONG_COUNTS_DEFINED
#define SIRKIT_TOURNAMENT_STRONG_COUNTS_DEFINED

namespace detail
{
struct TournamentSeries
{
    std::vector<std::uint32_t> all, strong;
};

inline TournamentSeries tournament_series(int n)
{
    if (n < 0 || static_cast<std::size_t>(n) >= FormalPowerSeries::max_precision())
    {
        throw std::length_error("tournament_series: vertex count outside FPS precision");
    }
    const auto size = static_cast<std::size_t>(n) + 1;
    std::vector<std::uint32_t> all(size, 1);
    std::uint32_t power_of_two = 1, factorial = 1;
    for (std::size_t k = 1; k < size; ++k)
    {
        all[k] = fps_multiply(all[k - 1], power_of_two);
        power_of_two = fps_add(power_of_two, power_of_two);
        factorial = fps_multiply(factorial, static_cast<std::uint32_t>(k));
    }
    auto inverse_factorial = fps_power(factorial, fps_modulus - 2);
    for (std::size_t k = size - 1; k != 0; --k)
    {
        all[k] = fps_multiply(all[k], inverse_factorial);
        inverse_factorial = fps_multiply(inverse_factorial, static_cast<std::uint32_t>(k));
    }

    const auto reciprocal = FormalPowerSeries(all).inverse();
    auto strong = reciprocal.coefficients();
    strong[0] = 0;
    for (std::size_t k = 1; k < size; ++k)
    {
        strong[k] = fps_subtract(0, strong[k]);
    }
    return {std::move(all), std::move(strong)};
}
} // namespace detail

[[nodiscard]] inline std::vector<std::uint32_t> tournament_strong_counts(int n)
{
    auto strong = detail::tournament_series(n).strong;
    std::uint32_t factorial = 1;
    for (std::size_t k = 1; k < strong.size(); ++k)
    {
        factorial = detail::fps_multiply(factorial, static_cast<std::uint32_t>(k));
        strong[k] = detail::fps_multiply(strong[k], factorial);
    }
    return strong;
}

[[nodiscard]] inline std::vector<std::uint32_t> tournament_sink_size_probabilities(int n)
{
    if (n == 0)
    {
        return {1};
    }
    auto [all, strong] = detail::tournament_series(n);
    const auto inv_total = detail::fps_power(all.back(), detail::fps_modulus - 2);
    for (std::size_t k = 1; k < all.size(); ++k)
    {
        strong[k] = detail::fps_multiply(
            detail::fps_multiply(strong[k], all[all.size() - 1 - k]), inv_total);
    }
    return strong;
}

#endif
