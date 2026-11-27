#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_GEOMETRIC_POLYNOMIAL_DEFINED
#define SIRKIT_GEOMETRIC_POLYNOMIAL_DEFINED

namespace reinsirk::detail
{
inline constexpr auto kGeometricPolynomialMod = NTT<>::modulus();
inline constexpr auto kGeometricPolynomialLength = std::size_t{1} << 20;

inline std::uint32_t geometric_polynomial_mul(std::uint32_t a, std::uint32_t b) noexcept
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kGeometricPolynomialMod);
}

inline std::uint32_t geometric_polynomial_sub(std::uint32_t a, std::uint32_t b) noexcept
{
    return a >= b ? a - b : a + kGeometricPolynomialMod - b;
}

inline std::uint32_t geometric_polynomial_power(std::uint32_t a, std::size_t exponent) noexcept
{
    std::uint32_t result = 1;
    for (; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1) != 0)
        {
            result = geometric_polynomial_mul(result, a);
        }
        a = geometric_polynomial_mul(a, a);
    }
    return result;
}

inline std::uint32_t geometric_polynomial_inverse(std::uint32_t a) noexcept
{
    return geometric_polynomial_power(a, kGeometricPolynomialMod - 2);
}

inline std::vector<std::uint32_t> geometric_nonzero_evaluation(
    NTT<> &ntt, std::span<const std::uint32_t> coefficients, std::uint32_t a,
    std::uint32_t ratio, std::uint32_t inverse_ratio, std::size_t count)
{
    // r^(ij)=r^(choose(i+j,2)-choose(i,2)-choose(j,2)) を畳み込みにする。
    std::vector<std::uint32_t> first(coefficients.size());
    std::vector<std::uint32_t> second(coefficients.size() + count - 1);
    std::uint32_t chirp = 1, step = 1;
    for (auto &value : second)
    {
        value = chirp;
        chirp = geometric_polynomial_mul(chirp, step);
        step = geometric_polynomial_mul(step, ratio);
    }
    chirp = 1;
    step = 1;
    std::uint32_t power_a = 1;
    for (std::size_t i = 0; i < first.size(); ++i)
    {
        first[first.size() - 1 - i] = geometric_polynomial_mul(coefficients[i] % kGeometricPolynomialMod,
                                                               geometric_polynomial_mul(chirp, power_a));
        power_a = geometric_polynomial_mul(power_a, a);
        chirp = geometric_polynomial_mul(chirp, step);
        step = geometric_polynomial_mul(step, inverse_ratio);
    }
    const auto product = ntt.convolution_normalized(std::move(first), std::move(second));
    std::vector<std::uint32_t> output(count);
    chirp = 1;
    step = 1;
    for (std::size_t i = 0; i < count; ++i)
    {
        output[i] = geometric_polynomial_mul(product[coefficients.size() - 1 + i], chirp);
        chirp = geometric_polynomial_mul(chirp, step);
        step = geometric_polynomial_mul(step, inverse_ratio);
    }
    return output;
}
}

[[nodiscard]] inline std::vector<std::uint32_t> evaluate_geometric_polynomial(
    std::span<const std::uint32_t> coefficients, std::uint64_t first, std::uint64_t common_ratio, std::size_t count)
{
    constexpr auto kMaxLength = ::reinsirk::detail::kGeometricPolynomialLength;
    if (coefficients.size() > kMaxLength || count > kMaxLength)
    {
        throw std::length_error("evaluate_geometric_polynomial: too many coefficients or points");
    }
    if (count == 0)
    {
        return {};
    }
    if (coefficients.empty())
    {
        return std::vector<std::uint32_t>(count);
    }
    const auto a = static_cast<std::uint32_t>(first % ::reinsirk::detail::kGeometricPolynomialMod);
    const auto ratio = static_cast<std::uint32_t>(common_ratio % ::reinsirk::detail::kGeometricPolynomialMod);
    const auto constant = coefficients.front() % ::reinsirk::detail::kGeometricPolynomialMod;
    if (a == 0)
    {
        return std::vector<std::uint32_t>(count, constant);
    }
    if (ratio <= 1)
    {
        std::uint32_t at_first = 0;
        for (auto i = coefficients.rbegin(); i != coefficients.rend(); ++i)
        {
            auto sum = ::reinsirk::detail::geometric_polynomial_mul(at_first, a) + *i % ::reinsirk::detail::kGeometricPolynomialMod;
            at_first = sum >= ::reinsirk::detail::kGeometricPolynomialMod ? sum - ::reinsirk::detail::kGeometricPolynomialMod : sum;
        }
        auto output = std::vector<std::uint32_t>(count, ratio == 0 ? constant : at_first);
        output.front() = at_first;
        return output;
    }
    NTT<> ntt;
    return ::reinsirk::detail::geometric_nonzero_evaluation(ntt, coefficients, a, ratio, ::reinsirk::detail::geometric_polynomial_inverse(ratio), count);
}

[[nodiscard]] inline std::optional<std::vector<std::uint32_t>> interpolate_geometric_polynomial(
    std::span<const std::uint32_t> values, std::uint64_t first, std::uint64_t common_ratio)
{
    const auto n = values.size();
    constexpr auto kMaxLength = ::reinsirk::detail::kGeometricPolynomialLength;
    if (n > kMaxLength)
    {
        throw std::length_error("interpolate_geometric_polynomial: too many points");
    }
    if (n == 0)
    {
        return std::vector<std::uint32_t>{};
    }
    if (n == 1)
    {
        return std::vector<std::uint32_t>{values.front() % ::reinsirk::detail::kGeometricPolynomialMod};
    }
    const auto a = static_cast<std::uint32_t>(first % ::reinsirk::detail::kGeometricPolynomialMod);
    const auto ratio = static_cast<std::uint32_t>(common_ratio % ::reinsirk::detail::kGeometricPolynomialMod);
    if (a == 0)
    {
        return std::nullopt;
    }
    const auto inverse_a = ::reinsirk::detail::geometric_polynomial_inverse(a);
    if (ratio == 0)
    {
        if (n != 2)
        {
            return std::nullopt;
        }
        const auto constant = values[1] % ::reinsirk::detail::kGeometricPolynomialMod;
        const auto difference = ::reinsirk::detail::geometric_polynomial_sub(values[0] % ::reinsirk::detail::kGeometricPolynomialMod, constant);
        return std::vector<std::uint32_t>{constant, ::reinsirk::detail::geometric_polynomial_mul(difference, inverse_a)};
    }

    std::vector<std::uint32_t> powers(n + 1, 1), factorial(n, 1), inverse_factorial(n);
    for (std::size_t i = 1; i < n; ++i)
    {
        powers[i] = ::reinsirk::detail::geometric_polynomial_mul(powers[i - 1], ratio);
        if (powers[i] == 1)
        {
            return std::nullopt;
        }
        factorial[i] = ::reinsirk::detail::geometric_polynomial_mul(factorial[i - 1], ::reinsirk::detail::geometric_polynomial_sub(1, powers[i]));
    }
    powers[n] = ::reinsirk::detail::geometric_polynomial_mul(powers[n - 1], ratio);
    inverse_factorial[n - 1] = ::reinsirk::detail::geometric_polynomial_inverse(factorial[n - 1]);
    for (auto i = n - 1; i != 0; --i)
    {
        inverse_factorial[i - 1] = ::reinsirk::detail::geometric_polynomial_mul(inverse_factorial[i], ::reinsirk::detail::geometric_polynomial_sub(1, powers[i]));
    }

    // q(x)=f(ax)。weight[i]は点r^iでのLagrangeの重み。
    std::vector<std::uint32_t> weights(n), reversed_node_polynomial(n, 1);
    const auto inverse_ratio = ::reinsirk::detail::geometric_polynomial_inverse(ratio);
    std::uint32_t power = 1, step = ::reinsirk::detail::geometric_polynomial_power(inverse_ratio, n - 2);
    for (std::size_t i = 0; i < n; ++i)
    {
        const auto inverse = ::reinsirk::detail::geometric_polynomial_mul(inverse_factorial[i], inverse_factorial[n - 1 - i]);
        auto value = ::reinsirk::detail::geometric_polynomial_mul(values[i] % ::reinsirk::detail::kGeometricPolynomialMod, ::reinsirk::detail::geometric_polynomial_mul(power, inverse));
        weights[i] = i % 2 == 0 ? value : ::reinsirk::detail::geometric_polynomial_sub(0, value);
        power = ::reinsirk::detail::geometric_polynomial_mul(power, step);
        step = ::reinsirk::detail::geometric_polynomial_mul(step, ratio);
    }
    // 積 ∏(1-r^i*x) の先頭n係数をq二項係数の比で列挙する。
    for (std::size_t i = 1; i < n; ++i)
    {
        const auto inverse = ::reinsirk::detail::geometric_polynomial_mul(factorial[i - 1], inverse_factorial[i]);
        const auto numerator = ::reinsirk::detail::geometric_polynomial_mul(powers[i - 1], ::reinsirk::detail::geometric_polynomial_sub(1, powers[n - i + 1]));
        const auto scale = ::reinsirk::detail::geometric_polynomial_mul(numerator, inverse);
        reversed_node_polynomial[i] = ::reinsirk::detail::geometric_polynomial_sub(0, ::reinsirk::detail::geometric_polynomial_mul(reversed_node_polynomial[i - 1], scale));
    }

    NTT<> ntt;
    auto moments = ::reinsirk::detail::geometric_nonzero_evaluation(ntt, weights, 1, ratio, inverse_ratio, n);
    auto coefficients = ntt.convolution_normalized(std::move(reversed_node_polynomial), std::move(moments));
    coefficients.resize(n);
    std::reverse(coefficients.begin(), coefficients.end());
    std::uint32_t scale = 1;
    for (auto &coefficient : coefficients)
    {
        coefficient = ::reinsirk::detail::geometric_polynomial_mul(coefficient, scale);
        scale = ::reinsirk::detail::geometric_polynomial_mul(scale, inverse_a);
    }
    return coefficients;
}
#endif
