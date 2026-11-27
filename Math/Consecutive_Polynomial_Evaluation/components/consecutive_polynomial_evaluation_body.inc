#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_CONSECUTIVE_POLYNOMIAL_EVALUATION_DEFINED
#define SIRKIT_CONSECUTIVE_POLYNOMIAL_EVALUATION_DEFINED
namespace reinsirk::detail
{
inline constexpr std::uint32_t kConsecutiveMod = NTT<>::modulus();

inline std::uint32_t ConsecutiveMultiply(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kConsecutiveMod);
}

inline std::uint32_t ConsecutiveInverse(std::uint32_t a)
{
    std::uint32_t result = 1;
    for (auto exp = kConsecutiveMod - 2; exp != 0; exp >>= 1)
    {
        if ((exp & 1U) != 0)
        {
            result = ConsecutiveMultiply(result, a);
        }
        a = ConsecutiveMultiply(a, a);
    }
    return result;
}

inline std::vector<std::uint32_t> ConsecutiveWeights(std::span<const std::uint32_t> samples)
{
    const auto n = samples.size();
    std::vector<std::uint32_t> inverse_factorial(n, 1);
    for (std::size_t i = 1; i < n; ++i)
    {
        inverse_factorial[i] = ConsecutiveMultiply(inverse_factorial[i - 1], static_cast<std::uint32_t>(i));
    }
    auto inverse = ConsecutiveInverse(inverse_factorial.back());
    for (std::size_t i = n; i > 1; --i)
    {
        inverse_factorial[i - 1] = inverse;
        inverse = ConsecutiveMultiply(inverse, static_cast<std::uint32_t>(i - 1));
    }
    std::vector<std::uint32_t> weighted(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        auto value = ConsecutiveMultiply(samples[i], ConsecutiveMultiply(inverse_factorial[i], inverse_factorial[n - 1 - i]));
        if ((n - 1 - i) % 2 != 0 && value != 0)
        {
            value = kConsecutiveMod - value;
        }
        weighted[i] = value;
    }
    return weighted;
}

inline void EvaluateConsecutiveOutside(NTT<> &ntt, std::uint32_t first, const std::vector<std::uint32_t> &weighted,
                                       std::span<std::uint32_t> output)
{
    const auto n = weighted.size();
    const auto lower = first - static_cast<std::uint32_t>(n) + 1;
    std::vector<std::uint32_t> inverse(n + output.size() - 1);
    std::uint32_t product = 1;
    for (std::size_t i = 0; i < inverse.size(); ++i)
    {
        product = ConsecutiveMultiply(product, lower + static_cast<std::uint32_t>(i));
        inverse[i] = product;
    }
    auto reciprocal = ConsecutiveInverse(product);
    product = inverse[n - 1];
    for (std::size_t i = inverse.size(); i > 0; --i)
    {
        inverse[i - 1] = ConsecutiveMultiply(reciprocal, i == 1 ? 1 : inverse[i - 2]);
        reciprocal = ConsecutiveMultiply(reciprocal, lower + static_cast<std::uint32_t>(i - 1));
    }
    // Lagrange補間の分母列と重みを畳み込み、分子は隣接する評価点へ差分更新する。
    const auto convolution = ntt.convolution_normalized(weighted, inverse);
    for (std::size_t i = 0; i < output.size(); ++i)
    {
        output[i] = ConsecutiveMultiply(convolution[n - 1 + i], product);
        product = ConsecutiveMultiply(product, ConsecutiveMultiply(first + static_cast<std::uint32_t>(i) + 1, inverse[i]));
    }
}
}

inline std::vector<std::uint32_t> evaluate_consecutive_polynomial(std::span<const std::uint32_t> samples,
                                                                  std::int64_t first, std::size_t count)
{
    constexpr auto kMaxSize = std::size_t{1} << 20;
    constexpr auto kMod = ::reinsirk::detail::kConsecutiveMod;
    if (samples.size() > kMaxSize || count > kMaxSize)
    {
        throw std::length_error("evaluate_consecutive_polynomial: expected at most 2^20 points");
    }
    if (samples.size() <= 1)
    {
        return std::vector<std::uint32_t>(count, samples.empty() ? 0 : samples[0] % kMod);
    }
    const auto residue = first % static_cast<std::int64_t>(kMod);
    auto x = static_cast<std::uint32_t>(residue < 0 ? residue + kMod : residue);
    std::vector<std::uint32_t> weighted, result(count);
    NTT<> ntt;
    for (std::size_t done = 0; done < count;)
    {
        if (x < samples.size())
        {
            const auto direct = std::min(count - done, samples.size() - x);
            for (std::size_t i = 0; i < direct; ++i)
            {
                result[done + i] = samples[x + i] % kMod;
            }
            x += static_cast<std::uint32_t>(direct);
            done += direct;
        }
        else
        {
            if (weighted.empty())
            {
                weighted = ::reinsirk::detail::ConsecutiveWeights(samples);
            }
            const auto outside = std::min(count - done, static_cast<std::size_t>(kMod - x));
            ::reinsirk::detail::EvaluateConsecutiveOutside(ntt, x, weighted, std::span<std::uint32_t>(result).subspan(done, outside));
            x += static_cast<std::uint32_t>(outside);
            done += outside;
            if (x == kMod)
            {
                x = 0;
            }
        }
    }
    return result;
}
#endif
