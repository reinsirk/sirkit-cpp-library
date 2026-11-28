// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#ifndef SIRKIT_FAST_FACTORIAL_DEFINED
#define SIRKIT_FAST_FACTORIAL_DEFINED
namespace reinsirk::detail
{
inline std::vector<std::uint32_t> FactorialBlockPrefixes(std::uint32_t width, std::uint32_t count)
{
    constexpr std::uint32_t kChunkSize = 1U << 20;
    const auto inverse_width = ConsecutiveInverse(width);
    std::vector<std::uint32_t> samples{1, width + 1};
    // P_d(x) = (Bx+1)...(Bx+d)、P_{2d}(x) = P_d(x)P_d(x+d/B)。
    for (std::uint32_t degree = 1; degree < width; degree *= 2)
    {
        const auto size = static_cast<std::size_t>(2 * degree + 1);
        const auto shifted = evaluate_consecutive_polynomial(samples, ConsecutiveMultiply(degree, inverse_width), size);
        samples = evaluate_consecutive_polynomial(samples, 0, size);
        for (std::size_t i = 0; i < size; ++i)
        {
            samples[i] = ConsecutiveMultiply(samples[i], shifted[i]);
        }
    }
    std::vector<std::uint32_t> prefixes;
    prefixes.reserve(count + 1);
    prefixes.push_back(1);
    for (std::uint32_t first = 0; first < count; first += kChunkSize)
    {
        const auto products = evaluate_consecutive_polynomial(samples, first, std::min(count - first, kChunkSize));
        for (const auto product : products)
        {
            prefixes.push_back(ConsecutiveMultiply(prefixes.back(), product));
        }
    }
    return prefixes;
}
}

inline std::vector<std::uint32_t> factorials_mod_998244353(std::span<const std::uint64_t> queries)
{
    constexpr auto kMod = ::reinsirk::detail::kConsecutiveMod;
    constexpr std::uint32_t kMinBlockWidth = 256;
    constexpr std::uint32_t kDirectLimit = 8192;
    struct Query
    {
        std::uint32_t value;
        std::size_t index;
    };
    std::vector<std::uint32_t> result(queries.size());
    std::vector<Query> ordered;
    ordered.reserve(queries.size());
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        if (queries[i] < kMod)
        {
            const auto n = static_cast<std::uint32_t>(queries[i]);
            ordered.push_back({std::min(n, kMod - 1 - n), i});
        }
    }
    if (ordered.empty())
    {
        return result;
    }
    std::sort(ordered.begin(), ordered.end(), [](const Query &a, const Query &b)
              {
                  return a.value < b.value;
              });
    const auto maximum = ordered.back().value;
    std::uint32_t width = kMod;
    std::vector<std::uint32_t> prefixes;
    if (maximum >= kDirectLimit)
    {
        // 評価点数とブロック内部を走査する回数の両方を抑える。
        const auto estimate = static_cast<std::uint32_t>(std::sqrt(8.0 * maximum / static_cast<double>(ordered.size())));
        width = std::max(kMinBlockWidth, std::bit_ceil(estimate));
        prefixes = ::reinsirk::detail::FactorialBlockPrefixes(width, maximum / width);
    }
    std::uint32_t block = 0;
    std::uint32_t previous = 0;
    std::uint32_t factorial = 1;
    for (const auto &[value, index] : ordered)
    {
        if (const auto next_block = value / width; next_block != block)
        {
            block = next_block;
            previous = block * width;
            factorial = prefixes[block];
        }
        for (; previous < value; ++previous)
        {
            factorial = ::reinsirk::detail::ConsecutiveMultiply(factorial, previous + 1);
        }
        if (queries[index] == value)
        {
            result[index] = factorial;
        }
        else
        {
            const auto inverse = ::reinsirk::detail::ConsecutiveInverse(factorial);
            result[index] = (value & 1U) == 0 ? kMod - inverse : inverse;
        }
    }
    return result;
}

inline std::uint32_t factorial_mod_998244353(std::uint64_t n)
{
    return factorials_mod_998244353(std::span<const std::uint64_t>(&n, 1)).front();
}
#endif
