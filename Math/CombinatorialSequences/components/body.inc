#include <span>

#ifndef SIRKIT_COMBINATORIAL_SEQUENCES_DEFINED
#define SIRKIT_COMBINATORIAL_SEQUENCES_DEFINED
namespace reinsirk::detail
{
inline void CheckCombinatorialDegree(std::size_t n)
{
    if (n >= FormalPowerSeries::max_precision())
    {
        throw std::length_error("combinatorial sequences: degree exceeds NTT budget");
    }
}

inline std::vector<std::uint32_t> CombinatorialInverseFactorials(std::size_t n)
{
    fps_prepare_inverses(n);
    const auto &inverse = fps_integer_inverses();
    std::vector<std::uint32_t> factorial(n + 1, 1);
    for (std::size_t i = 1; i <= n; ++i)
    {
        factorial[i] = fps_multiply(factorial[i - 1], inverse[i]);
    }
    return factorial;
}

inline FormalPowerSeries FallingFactorial(std::size_t first, std::size_t last)
{
    if (last - first <= 32)
    {
        std::vector<std::uint32_t> a(last - first + 1);
        a[0] = 1;
        for (auto root = first; root < last; ++root)
        {
            const auto degree = root - first;
            const auto r = static_cast<std::uint32_t>(root);
            for (std::size_t i = degree + 1; i > 0; --i)
            {
                a[i] = fps_subtract(a[i - 1], fps_multiply(r, a[i]));
            }
            a[0] = fps_subtract(0, fps_multiply(r, a[0]));
        }
        return FormalPowerSeries(std::move(a));
    }
    const auto mid = first + (last - first) / 2;
    return FallingFactorial(first, mid) * FallingFactorial(mid, last);
}
}

inline std::vector<std::uint32_t> partition_numbers(std::size_t n)
{
    ::reinsirk::detail::CheckCombinatorialDegree(n);
    std::vector<std::uint32_t> euler(n + 1);
    euler[0] = 1;
    // 五角数定理で prod(1-x^k) の非零係数だけを置く。
    constexpr auto kNegative = FormalPowerSeries::modulus() - 1;
    for (std::size_t k = 1, first = 1; first <= n; first += 3 * k + 1, ++k)
    {
        const auto sign = k % 2 == 0 ? 1U : kNegative;
        euler[first] = sign;
        if (first + k <= n)
        {
            euler[first + k] = sign;
        }
    }
    return FormalPowerSeries(std::move(euler)).inverse(n + 1).coefficients();
}

inline std::vector<std::uint32_t> bell_numbers(std::size_t n)
{
    ::reinsirk::detail::CheckCombinatorialDegree(n);
    auto inverse_factorials = ::reinsirk::detail::CombinatorialInverseFactorials(n);
    // 組合せ種「集合の集合」の指数母関数は exp(exp(x)-1)。
    inverse_factorials[0] = 0;
    const auto egf = FormalPowerSeries(std::move(inverse_factorials)).exp(n + 1);
    std::vector<std::uint32_t> result(n + 1);
    std::uint32_t factorial = 1;
    for (std::size_t k = 0; k <= n; ++k)
    {
        result[k] = ::reinsirk::detail::fps_multiply(egf.get(k), factorial);
        factorial = ::reinsirk::detail::fps_multiply(factorial, static_cast<std::uint32_t>(k + 1));
    }
    return result;
}

inline std::vector<std::uint32_t> stirling_first_row(std::size_t n)
{
    ::reinsirk::detail::CheckCombinatorialDegree(n);
    return ::reinsirk::detail::FallingFactorial(0, n).coefficients();
}

inline std::vector<std::uint32_t> stirling_second_row(std::size_t n)
{
    ::reinsirk::detail::CheckCombinatorialDegree(n);
    auto alternating = ::reinsirk::detail::CombinatorialInverseFactorials(n);
    std::vector<std::uint32_t> powered(n + 1);
    for (std::size_t i = 0; i <= n; ++i)
    {
        powered[i] = ::reinsirk::detail::fps_multiply(alternating[i],
                                                      ::reinsirk::detail::fps_power(static_cast<std::uint32_t>(i), n));
        if (i % 2 != 0)
        {
            alternating[i] = FormalPowerSeries::modulus() - alternating[i];
        }
    }
    // 全射の包除和を i! で割り、列全体を畳み込む。
    auto coefficients = ::reinsirk::detail::fps_engine().convolution_normalized(std::move(alternating), std::move(powered));
    coefficients.resize(n + 1);
    return coefficients;
}

inline std::vector<std::uint32_t> bernoulli_numbers(std::size_t n)
{
    ::reinsirk::detail::CheckCombinatorialDegree(n);
    auto shifted_exp = ::reinsirk::detail::CombinatorialInverseFactorials(n + 1);
    shifted_exp.erase(shifted_exp.begin());
    auto result = FormalPowerSeries(std::move(shifted_exp)).inverse(n + 1).coefficients();
    std::uint32_t factorial = 1;
    for (std::size_t i = 1; i <= n; ++i)
    {
        factorial = ::reinsirk::detail::fps_multiply(factorial, static_cast<std::uint32_t>(i));
        result[i] = ::reinsirk::detail::fps_multiply(result[i], factorial);
    }
    return result;
}

inline std::uint32_t sum_of_powers(std::uint64_t n, std::size_t exponent)
{
    constexpr auto kMod = FormalPowerSeries::modulus();
    if (exponent == 0)
    {
        return static_cast<std::uint32_t>(n % kMod);
    }
    const auto bernoulli = bernoulli_numbers(exponent);
    const auto &inverse = ::reinsirk::detail::fps_integer_inverses();
    const auto x = ::reinsirk::detail::fps_add(static_cast<std::uint32_t>(n % kMod), 1);
    std::uint32_t binomial = 1;
    std::uint32_t sum = 0;
    // [B_{k+1}(n+1) - B_{k+1}(0)] / (k+1) をHorner法で評価する。
    for (std::size_t j = 0; j <= exponent; ++j)
    {
        const auto term = ::reinsirk::detail::fps_multiply(binomial, bernoulli[j]);
        sum = ::reinsirk::detail::fps_add(::reinsirk::detail::fps_multiply(sum, x), term);
        binomial = ::reinsirk::detail::fps_multiply(binomial, static_cast<std::uint32_t>(exponent + 1 - j));
        binomial = ::reinsirk::detail::fps_multiply(binomial, inverse[j + 1]);
    }
    return ::reinsirk::detail::fps_multiply(::reinsirk::detail::fps_multiply(sum, x), inverse[exponent + 1]);
}

inline std::vector<std::uint32_t> derangement_numbers(std::size_t n, std::uint32_t mod = 998244353)
{
    if (mod == 0)
    {
        throw std::invalid_argument("derangement_numbers: zero modulus");
    }
    if (n >= std::vector<std::uint32_t>().max_size())
    {
        throw std::length_error("derangement_numbers: size exceeds capacity");
    }
    std::vector<std::uint32_t> result(n + 1);
    result[0] = 1 % mod;
    for (std::size_t k = 2; k <= n; ++k)
    {
        const auto previous = (std::uint64_t{result[k - 1]} + result[k - 2]) % mod;
        result[k] = static_cast<std::uint32_t>((k - 1) % mod * previous % mod);
    }
    return result;
}

inline std::vector<std::uint32_t> subset_sum_counts(std::span<const std::size_t> values, std::size_t max_sum)
{
    ::reinsirk::detail::CheckCombinatorialDegree(max_sum);
    std::vector<std::uint32_t> frequency(max_sum + 1);
    std::size_t zeros = 0;
    for (const auto value : values)
    {
        if (value == 0)
        {
            ++zeros;
        }
        else if (value <= max_sum)
        {
            frequency[value] = ::reinsirk::detail::fps_add(frequency[value], 1);
        }
    }

    // 同じ値vを頻度でまとめて log((1+x^v)^frequency[v]) を加える。
    ::reinsirk::detail::fps_prepare_inverses(max_sum);
    const auto &inverses = ::reinsirk::detail::fps_integer_inverses();
    std::vector<std::uint32_t> logarithm(max_sum + 1);
    for (std::size_t value = 1; value <= max_sum; ++value)
    {
        if (frequency[value] == 0)
        {
            continue;
        }
        for (std::size_t k = 1, degree = value; degree <= max_sum; ++k, degree += value)
        {
            const auto term = ::reinsirk::detail::fps_multiply(frequency[value], inverses[k]);
            logarithm[degree] = k % 2 == 0 ? ::reinsirk::detail::fps_subtract(logarithm[degree], term)
                                           : ::reinsirk::detail::fps_add(logarithm[degree], term);
        }
    }
    auto result = FormalPowerSeries(std::move(logarithm)).exp(max_sum + 1).coefficients();
    if (zeros != 0)
    {
        const auto scale = ::reinsirk::detail::fps_power(2, zeros);
        for (auto &coefficient : result)
        {
            coefficient = ::reinsirk::detail::fps_multiply(coefficient, scale);
        }
    }
    return result;
}
#endif
