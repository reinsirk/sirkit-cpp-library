// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Subset_Sum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subset_sum.md
//
// API一覧
// subset_sum(values,target): 和がtargetになる部分集合の添字を昇順で1組返す。なければnullopt。
// valuesは非負整数。各添字を高々1回選ぶ。target=0では空集合。
// 64ビットごとの到達状態を使い、O(n(1+target/64)+target)時間・O(target+1)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBSET_SUM_DEFINED
#define SIRKIT_SUBSET_SUM_DEFINED

inline std::optional<std::vector<std::size_t>> subset_sum(std::span<const std::size_t> values, std::size_t target)
{
    std::vector<std::size_t> result;
    if (target == 0)
    {
        return result;
    }
    std::size_t possible = 0;
    for (auto value : values)
    {
        if (value <= target)
        {
            possible += std::min(value, target - possible);
        }
    }
    if (possible < target)
    {
        return std::nullopt;
    }
    if (target >= result.max_size())
    {
        throw std::length_error("subset_sum: target too large");
    }

    std::vector<std::size_t> choice(target + 1, values.size());
    constexpr unsigned kBits = 64;
    constexpr auto kOnes = std::numeric_limits<std::uint64_t>::max();
    const auto high = target / kBits;
    const auto last_mask = kOnes >> (kBits - 1 - target % kBits);
    std::vector<std::uint64_t> reachable(high + 1);
    reachable[0] = 1;
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        const auto value = values[i];
        if (value == 0 || value > target)
        {
            continue;
        }
        const auto whole = value / kBits;
        const auto bits = value % kBits;
        // 上位から移すことで、同じ要素を2回使わない。
        for (auto word = high + 1; word-- > whole;)
        {
            auto added = reachable[word - whole] << bits;
            if (bits != 0 && word > whole)
            {
                added |= reachable[word - whole - 1] >> (kBits - bits);
            }
            if (word == high)
            {
                added &= last_mask;
            }
            auto fresh = added & ~reachable[word];
            reachable[word] |= added;
            while (fresh != 0)
            {
                const auto sum = word * kBits + std::countr_zero(fresh);
                choice[sum] = i;
                fresh &= fresh - 1;
            }
        }
        if (choice[target] != values.size())
        {
            for (auto sum = target; sum != 0;)
            {
                const auto index = choice[sum];
                result.push_back(index);
                sum -= values[index];
            }
            std::ranges::reverse(result);
            return result;
        }
    }
    return std::nullopt;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
