// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Exponential_Polynomial_Sum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/exponential_polynomial_sum.md
//
// API一覧
// sum_exponential_polynomial(samples,r,n): f(0)..f(d)を渡し、i=0..n-1のr^i*f(i)の和を法998244353で返す。
// sum_exponential_polynomial_limit(samples,r): 同じ和の有理関数としての無限和。r≡1ならnullopt。
// sum_exponential_powers(r,d,n)/sum_exponential_powers_limit(r,d): f(i)=i^d。0^0=1。
// 空のsamplesはf=0、非空ならd+1<998244353。r,nはuint64_t。次数dについてO(d+1+log(n+1))時間・O(d+1)領域。
// べき乗版はi^dを線形篩で作る。整数剰余演算をO(1)として同じ計算量。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EXPONENTIAL_POLYNOMIAL_SUM_DEFINED
#define SIRKIT_EXPONENTIAL_POLYNOMIAL_SUM_DEFINED
namespace detail
{
inline constexpr std::uint32_t kExponentialSumMod = 998244353;

inline std::uint32_t exponential_sum_add(std::uint32_t a, std::uint32_t b) noexcept
{
    const auto sum = a + b;
    return sum < kExponentialSumMod ? sum : sum - kExponentialSumMod;
}

inline std::uint32_t exponential_sum_subtract(std::uint32_t a, std::uint32_t b) noexcept
{
    return a >= b ? a - b : a + kExponentialSumMod - b;
}

inline std::uint32_t exponential_sum_multiply(std::uint32_t a, std::uint32_t b) noexcept
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kExponentialSumMod);
}

inline std::uint32_t exponential_sum_power(std::uint32_t a, std::uint64_t n) noexcept
{
    std::uint32_t result = 1;
    for (; n != 0; n >>= 1)
    {
        if (n & 1)
        {
            result = exponential_sum_multiply(result, a);
        }
        a = exponential_sum_multiply(a, a);
    }
    return result;
}

inline void check_exponential_sample_count(std::size_t count)
{
    if (count >= kExponentialSumMod)
    {
        throw std::length_error("exponential polynomial sum: expected fewer samples than the modulus");
    }
}

struct ExponentialFactorials
{
    std::vector<std::uint32_t> inverse;
    std::uint32_t factorial;

    explicit ExponentialFactorials(std::size_t degree) : inverse(degree + 1), factorial(1)
    {
        for (std::uint32_t i = 1; i <= degree; ++i)
        {
            factorial = exponential_sum_multiply(factorial, i);
        }
        inverse[degree] = exponential_sum_power(factorial, kExponentialSumMod - 2);
        for (auto i = degree; i != 0; --i)
        {
            inverse[i - 1] = exponential_sum_multiply(inverse[i], static_cast<std::uint32_t>(i));
        }
    }
};

inline void check_exponential_power_degree(std::size_t degree)
{
    if (degree >= kExponentialSumMod - 1)
    {
        throw std::length_error("exponential power sum: degree too large");
    }
}

inline std::vector<std::uint32_t> exponential_power_samples(std::size_t degree, std::size_t count)
{
    if (degree == 0)
    {
        return std::vector<std::uint32_t>(count, 1);
    }
    std::vector<std::uint32_t> result(count);
    if (count < 2)
    {
        return result;
    }
    std::vector<std::uint32_t> primes;
    result[1] = 1;
    for (std::uint32_t i = 2; i < count; ++i)
    {
        if (result[i] == 0)
        {
            result[i] = exponential_sum_power(i, degree);
            primes.push_back(i);
        }
        // primesにはiの最小素因数がある。そこまで、または積が上限に達するまで取る。
        for (std::size_t j = 0;; ++j)
        {
            const auto prime = primes[j];
            const auto next = std::uint64_t{i} * prime;
            if (next >= count)
            {
                break;
            }
            result[next] = exponential_sum_multiply(result[i], result[prime]);
            if (i % prime == 0)
            {
                break;
            }
        }
    }
    return result;
}

inline std::uint32_t exponential_direct(std::span<const std::uint32_t> samples, std::uint32_t ratio)
{
    std::uint32_t answer = 0, weight = 1;
    for (const auto value : samples)
    {
        answer = exponential_sum_add(answer, exponential_sum_multiply(value % kExponentialSumMod, weight));
        weight = exponential_sum_multiply(weight, ratio);
    }
    return answer;
}

inline void exponential_prefix_in_place(std::vector<std::uint32_t> &samples, std::uint32_t ratio)
{
    std::uint32_t answer = 0, weight = 1;
    for (auto &value : samples)
    {
        const auto current = exponential_sum_multiply(value % kExponentialSumMod, weight);
        value = answer;
        answer = exponential_sum_add(answer, current);
        weight = exponential_sum_multiply(weight, ratio);
    }
    samples.push_back(answer);
}

inline std::uint32_t exponential_interpolate(std::span<const std::uint32_t> samples,
                                             std::span<const std::uint32_t> inverse_factorial,
                                             std::uint32_t x)
{
    if (x < samples.size())
    {
        return samples[x];
    }
    const auto degree = samples.size() - 1;
    std::vector<std::uint32_t> prefix(samples.size(), 1);
    for (std::size_t i = 0; i < degree; ++i)
    {
        prefix[i + 1] = exponential_sum_multiply(prefix[i], x - static_cast<std::uint32_t>(i));
    }
    std::uint32_t suffix = 1, result = 0;
    for (auto j = samples.size(); j != 0; --j)
    {
        const auto i = j - 1;
        auto term = exponential_sum_multiply(samples[i], exponential_sum_multiply(prefix[i], suffix));
        term = exponential_sum_multiply(term, exponential_sum_multiply(inverse_factorial[i], inverse_factorial[degree - i]));
        result = (degree - i) % 2 == 0 ? exponential_sum_add(result, term) : exponential_sum_subtract(result, term);
        suffix = exponential_sum_multiply(suffix, x - static_cast<std::uint32_t>(i));
    }
    return result;
}

inline std::uint32_t exponential_constant(std::span<const std::uint32_t> prefix,
                                          const ExponentialFactorials &factorials,
                                          std::uint32_t inverse_ratio)
{
    const auto degree = prefix.size() - 1;
    const auto negate_inverse = kExponentialSumMod - inverse_ratio;
    std::uint32_t weight = 1, numerator = 0;
    for (std::size_t i = 0; i <= degree; ++i)
    {
        auto term = exponential_sum_multiply(prefix[i], weight);
        term = exponential_sum_multiply(term, exponential_sum_multiply(factorials.inverse[i], factorials.inverse[degree - i]));
        numerator = exponential_sum_add(numerator, term);
        weight = exponential_sum_multiply(weight, negate_inverse);
    }
    const auto denominator = exponential_sum_subtract(1, inverse_ratio);
    const auto reciprocal = exponential_sum_power(denominator, kExponentialSumMod - 1 - degree);
    return exponential_sum_multiply(exponential_sum_multiply(numerator, factorials.factorial), reciprocal);
}

inline std::uint32_t exponential_finite(std::vector<std::uint32_t> samples, std::uint32_t ratio,
                                        std::uint64_t n)
{
    if (samples.empty() || n == 0)
    {
        return 0;
    }
    if (ratio == 0)
    {
        return samples.front() % kExponentialSumMod;
    }
    const auto x = static_cast<std::uint32_t>(n % kExponentialSumMod);
    const auto terms = ratio == 1 ? x : n;
    if (terms <= samples.size())
    {
        return exponential_direct(std::span<const std::uint32_t>(samples).first(static_cast<std::size_t>(terms)), ratio);
    }

    const ExponentialFactorials factorials(samples.size());
    exponential_prefix_in_place(samples, ratio);
    if (ratio == 1)
    {
        return exponential_interpolate(samples, factorials.inverse, x);
    }

    // S(i)=C+r^i*q(i) でqは元のf以下の次数。qの次の有限差分が0となるようにCを求める。
    const auto inverse_ratio = exponential_sum_power(ratio, kExponentialSumMod - 2);
    const auto constant = exponential_constant(samples, factorials, inverse_ratio);
    samples.pop_back();
    std::uint32_t weight = 1;
    for (auto &value : samples)
    {
        value = exponential_sum_multiply(exponential_sum_subtract(value, constant), weight);
        weight = exponential_sum_multiply(weight, inverse_ratio);
    }
    const auto q = exponential_interpolate(samples, factorials.inverse, x);
    return exponential_sum_add(constant, exponential_sum_multiply(exponential_sum_power(ratio, n), q));
}

inline std::optional<std::uint32_t> exponential_limit(std::vector<std::uint32_t> samples, std::uint32_t ratio)
{
    if (ratio == 1)
    {
        return std::nullopt;
    }
    if (samples.empty())
    {
        return 0;
    }
    if (ratio == 0)
    {
        return samples.front() % kExponentialSumMod;
    }
    const ExponentialFactorials factorials(samples.size());
    exponential_prefix_in_place(samples, ratio);
    const auto inverse_ratio = exponential_sum_power(ratio, kExponentialSumMod - 2);
    return exponential_constant(samples, factorials, inverse_ratio);
}
} // namespace detail

[[nodiscard]] inline std::uint32_t sum_exponential_polynomial(std::span<const std::uint32_t> samples,
                                                              std::uint64_t ratio, std::uint64_t n)
{
    detail::check_exponential_sample_count(samples.size());
    return detail::exponential_finite({samples.begin(), samples.end()}, ratio % detail::kExponentialSumMod, n);
}

[[nodiscard]] inline std::optional<std::uint32_t> sum_exponential_polynomial_limit(
    std::span<const std::uint32_t> samples, std::uint64_t ratio)
{
    detail::check_exponential_sample_count(samples.size());
    return detail::exponential_limit({samples.begin(), samples.end()}, ratio % detail::kExponentialSumMod);
}

[[nodiscard]] inline std::uint32_t sum_exponential_powers(std::uint64_t ratio, std::size_t degree,
                                                          std::uint64_t n)
{
    detail::check_exponential_power_degree(degree);
    const auto r = static_cast<std::uint32_t>(ratio % detail::kExponentialSumMod);
    if (r == 0)
    {
        return n != 0 && degree == 0;
    }
    const auto terms = r == 1 ? n % detail::kExponentialSumMod : n;
    const auto count = degree + 1;
    if (terms <= count)
    {
        return detail::exponential_direct(detail::exponential_power_samples(degree, static_cast<std::size_t>(terms)), r);
    }
    return detail::exponential_finite(detail::exponential_power_samples(degree, count), r, n);
}

[[nodiscard]] inline std::optional<std::uint32_t> sum_exponential_powers_limit(std::uint64_t ratio,
                                                                               std::size_t degree)
{
    detail::check_exponential_power_degree(degree);
    const auto r = static_cast<std::uint32_t>(ratio % detail::kExponentialSumMod);
    if (r == 1)
    {
        return std::nullopt;
    }
    const auto count = r == 0 ? std::size_t{1} : degree + 1;
    return detail::exponential_limit(detail::exponential_power_samples(degree, count), r);
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
