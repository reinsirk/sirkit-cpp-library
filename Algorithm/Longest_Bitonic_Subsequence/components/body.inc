#include <algorithm>
#include <cstddef>
#include <vector>

#ifndef SIRKIT_LONGEST_BITONIC_SUBSEQUENCE_DEFINED
#define SIRKIT_LONGEST_BITONIC_SUBSEQUENCE_DEFINED

struct BitonicSubsequenceResult
{
    std::vector<std::size_t> indices;
    std::size_t peak = 0;
};

template <bool Strict = true, typename T>
[[nodiscard]] BitonicSubsequenceResult longest_bitonic_subsequence(const std::vector<T> &values)
{
    const std::size_t n = values.size();
    if (n == 0)
    {
        return {};
    }

    const auto from_left = LISEndingLengths<Strict>(values);
    const auto from_right = LISEndingLengths<Strict>(std::vector<T>(values.rbegin(), values.rend()));
    std::size_t peak = 0, best = 0;
    for (std::size_t i = 0; i < n; ++i)
    {
        const std::size_t length = from_left[i] + from_right[n - 1 - i] - 1;
        if (length > best)
        {
            peak = i;
            best = length;
        }
    }

    BitonicSubsequenceResult result;
    result.indices.reserve(best);
    result.indices.push_back(peak);
    // 目的の長さを持つ最後の候補なら、現在の要素へ単調に延長できる。
    std::size_t remaining = from_left[peak];
    for (std::size_t i = peak; i != 0 && remaining > 1;)
    {
        --i;
        if (from_left[i] + 1 == remaining)
        {
            result.indices.push_back(i);
            --remaining;
        }
    }
    std::ranges::reverse(result.indices);
    result.peak = result.indices.size() - 1;

    remaining = from_right[n - 1 - peak];
    for (std::size_t i = peak + 1; i < n && remaining > 1; ++i)
    {
        if (from_right[n - 1 - i] + 1 == remaining)
        {
            result.indices.push_back(i);
            --remaining;
        }
    }
    return result;
}

#endif
