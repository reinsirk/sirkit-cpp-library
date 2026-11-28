// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    const auto multiply = sirkit::detail::fps_multiply;
    const auto power = sirkit::detail::fps_power;
    constexpr auto kMod = sirkit::FormalPowerSeries::modulus();
    constexpr std::uint32_t kLength = 100000000;
    std::size_t n{}, m{};
    verify_in >> n >> m;
    const auto inverse_length = power(kLength, kMod - 2);
    const auto inverse_m = power(static_cast<std::uint32_t>(m), kMod - 2);
    std::vector<std::uint32_t> points(m), weights(m, inverse_m);
    for (auto &point : points)
    {
        std::uint32_t offset{};
        verify_in >> offset;
        point = sirkit::detail::fps_subtract(multiply(offset, inverse_length), 1);
    }

    const auto moments = sirkit::weighted_power_sums(points, weights, n + 1);
    sirkit::detail::fps_prepare_inverses(n);
    const auto &inverses = sirkit::detail::fps_integer_inverses();
    std::vector<std::uint32_t> average(n + 1);
    average[0] = 1;
    std::uint32_t inverse_factorial = 1;
    for (std::size_t i = 1; i <= n; ++i)
    {
        inverse_factorial = multiply(inverse_factorial, inverses[i]);
        average[i] = multiply(moments[i], inverse_factorial);
    }
    const auto reciprocal = sirkit::FormalPowerSeries(std::move(average)).inverse(n + 1);
    auto input = reciprocal;
    for (std::size_t i = 1; i <= n; ++i)
    {
        input.set(i, sirkit::detail::fps_subtract(reciprocal[i], reciprocal[i - 1]));
    }
    // V(x)g(x*exp(-x))=1 と変数変換の因子 (1-x) を合わせる。
    const auto volumes = sirkit::exponential_coefficient_transform(input, 1, n + 1);
    std::uint32_t factorial = 1;
    for (std::size_t i = 1; i <= n; ++i)
    {
        factorial = multiply(factorial, static_cast<std::uint32_t>(i));
        verify_out << multiply(multiply(volumes[i], factorial), power(inverses[i], i)) << '\n';
    }
}
