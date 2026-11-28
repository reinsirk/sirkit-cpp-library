#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_FERRERS_ROOK_COUNTS
#define SIRKIT_FERRERS_ROOK_COUNTS

namespace reinsirk::detail
{
[[nodiscard]] inline FormalPowerSeries ferrers_factor_product(std::span<const std::uint32_t> constants)
{
    if (constants.size() <= 1)
    {
        return constants.empty() ? FormalPowerSeries{1} : FormalPowerSeries{constants.front(), 1};
    }
    const auto middle = constants.size() / 2;
    return ferrers_factor_product(constants.first(middle)) * ferrers_factor_product(constants.subspan(middle));
}
} // namespace reinsirk::detail

[[nodiscard]] inline std::vector<std::uint32_t> ferrers_rook_counts(std::span<const std::uint32_t> heights)
{
    if (heights.size() >= (std::size_t{1} << 20))
    {
        throw std::length_error("ferrers_rook_counts: too many columns");
    }
    std::vector<std::uint32_t> constants(heights.begin(), heights.end());
    std::ranges::sort(constants);
    std::vector<std::uint32_t> points(heights.size());
    std::iota(points.begin(), points.end(), std::uint32_t{0});
    constexpr auto kMod = FormalPowerSeries::modulus();
    for (std::size_t i = 0; i < heights.size(); ++i)
    {
        constants[i] = ::reinsirk::detail::fps_subtract(constants[i] % kMod, points[i]);
    }
    // ∏(x+height[i]−i) の下降階乗係数は、次数を反転したルーク数。
    const auto product = ::reinsirk::detail::ferrers_factor_product(constants);
    auto result = monomial_to_newton_basis(product.coefficients(), points);
    std::ranges::reverse(result);
    return result;
}

[[nodiscard]] inline std::vector<std::uint32_t> ferrers_hit_counts(std::span<const std::uint32_t> heights)
{
    const auto n = heights.size();
    if (std::ranges::any_of(heights, [n](std::uint32_t height)
                            {
                                return height > n;
                            }))
    {
        throw std::invalid_argument("ferrers_hit_counts: column outside square board");
    }
    const auto rook = ferrers_rook_counts(heights);
    const auto multiply = ::reinsirk::detail::fps_multiply;
    const auto subtract = ::reinsirk::detail::fps_subtract;
    constexpr auto kMod = FormalPowerSeries::modulus();
    std::vector<std::uint32_t> factorial(n + 1, 1), inverse_factorial(n + 1);
    for (std::size_t i = 1; i <= n; ++i)
    {
        factorial[i] = multiply(factorial[i - 1], static_cast<std::uint32_t>(i));
    }
    inverse_factorial[n] = ::reinsirk::detail::fps_power(factorial[n], kMod - 2);
    for (std::size_t i = n; i != 0; --i)
    {
        inverse_factorial[i - 1] = multiply(inverse_factorial[i], static_cast<std::uint32_t>(i));
    }
    // ∑ rook[k](n-k)!(y-1)^k の単項式係数を階乗で重み付けし、畳み込みで求める。
    FormalPowerSeries left(n + 1), right(n + 1);
    for (std::size_t i = 0; i <= n; ++i)
    {
        left.set(n - i, multiply(multiply(rook[i], factorial[n - i]), factorial[i]));
        right.set(i, i % 2 == 0 ? inverse_factorial[i] : subtract(0, inverse_factorial[i]));
    }
    const auto product = left * right;
    std::vector<std::uint32_t> result(n + 1);
    for (std::size_t i = 0; i <= n; ++i)
    {
        result[i] = multiply(product[n - i], inverse_factorial[i]);
    }
    return result;
}
#endif
