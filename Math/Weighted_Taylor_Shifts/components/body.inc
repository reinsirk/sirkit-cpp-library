#include <concepts>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_WEIGHTED_TAYLOR_SHIFTS
#define SIRKIT_WEIGHTED_TAYLOR_SHIFTS

template <std::integral Shift, std::integral Weight>
[[nodiscard]] FormalPowerSeries weighted_taylor_shifts(const FormalPowerSeries &f, const std::vector<Shift> &shifts,
                                                       const std::vector<Weight> &weights)
{
    if (shifts.size() != weights.size())
    {
        throw std::invalid_argument("weighted_taylor_shifts: expected one weight per shift");
    }
    const auto n = f.size();
    detail::fps_require_precision(n);
    if (f.empty() || shifts.empty())
    {
        return FormalPowerSeries(n);
    }
    auto moments = weighted_power_sums(shifts, weights, n);
    const auto multiply = detail::fps_multiply;
    detail::fps_prepare_inverses(n - 1);
    const auto &inverse = detail::fps_integer_inverses();
    std::uint32_t factorial = 1;
    std::vector<std::uint32_t> inverse_factorial(n, 1), reversed(n);
    reversed.back() = f[0];
    // 出力j次の係数はΣ f[k]*k!*moment[k−j]/(k−j)! をj!で割った値。
    for (std::size_t i = 1; i < n; ++i)
    {
        factorial = multiply(factorial, static_cast<std::uint32_t>(i));
        inverse_factorial[i] = multiply(inverse_factorial[i - 1], inverse[i]);
        reversed[n - 1 - i] = multiply(f[i], factorial);
        moments.set(i, multiply(moments[i], inverse_factorial[i]));
    }
    const auto product = FormalPowerSeries(std::move(reversed)) * moments;
    std::vector<std::uint32_t> result(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        result[i] = multiply(product[n - 1 - i], inverse_factorial[i]);
    }
    return FormalPowerSeries(std::move(result));
}
#endif
