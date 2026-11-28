// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    constexpr auto kMod = sirkit::FormalPowerSeries::modulus();
    std::size_t types{}, moves{};
    verify_in >> types >> moves;
    std::uint32_t initial = 1;
    std::vector<std::uint32_t> ratios(types), multiplicities(types);
    for (std::size_t i = 0; i < types; ++i)
    {
        std::uint32_t front{}, back{};
        verify_in >> front >> back >> multiplicities[i];
        ratios[i] = sirkit::detail::fps_multiply(back, sirkit::detail::fps_power(front, kMod - 2));
        initial = sirkit::detail::fps_multiply(initial, sirkit::detail::fps_power(front, multiplicities[i]));
    }

    const auto moments = sirkit::weighted_power_sums(ratios, multiplicities, moves + 1);
    sirkit::detail::fps_prepare_inverses(moves);
    const auto &inverses = sirkit::detail::fps_integer_inverses();
    std::vector<std::uint32_t> logarithm(moves + 1);
    for (std::size_t i = 1; i <= moves; ++i)
    {
        const auto magnitude = sirkit::detail::fps_multiply(moments.get(i), inverses[i]);
        logarithm[i] = (i & 1u) != 0 ? magnitude : sirkit::detail::fps_subtract(0, magnitude);
    }
    const auto polynomial = sirkit::FormalPowerSeries(std::move(logarithm)).exp(moves + 1);
    const auto score = sirkit::exponential_composition(polynomial, 1, moves + 1, -1);
    std::uint32_t factorial = initial;
    for (std::size_t i = 1; i <= moves; ++i)
    {
        factorial = sirkit::detail::fps_multiply(factorial, static_cast<std::uint32_t>(i));
        verify_out << sirkit::detail::fps_multiply(score.get(i), factorial) << '\n';
    }
}
