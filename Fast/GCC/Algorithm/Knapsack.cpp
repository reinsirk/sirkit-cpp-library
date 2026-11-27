// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Knapsack.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/knapsack.md
//
// API一覧
// KnapsackItem {weight, value, count=1}: 重さ、非負の価値、選べる個数の上限。重さ0も可。
// maximize_knapsack(capacity,items): 合計重量capacity以下の最大価値を返す。
// maximize_exact_knapsack(target,items): 合計重量targetの最大価値。作れなければnullopt。
// 途中で作れる選択の価値の総和はint64_tに収める。超過はoverflow_error。
// 容量Wと品目数nに対して両方ともO((n+1)(W+1))時間・O(W+1)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_KNAPSACK_DEFINED
#define SIRKIT_KNAPSACK_DEFINED
struct KnapsackItem
{
    std::size_t weight = 0;
    std::int64_t value = 0;
    std::size_t count = 1;
};

namespace detail
{
inline std::int64_t KnapsackAdd(std::int64_t base, std::int64_t value, std::size_t count)
{
    if (value == 0)
    {
        return base;
    }
    if (count > static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max() - base) / value))
    {
        throw std::overflow_error("knapsack: value does not fit in int64_t");
    }
    return base + static_cast<std::int64_t>(count) * value;
}

inline std::optional<std::int64_t> SolveKnapsack(std::size_t capacity, std::span<const KnapsackItem> items, bool exact)
{
    if (capacity >= std::vector<std::int64_t>().max_size())
    {
        throw std::length_error("knapsack: capacity exceeds storage limit");
    }
    std::int64_t zero_weight = 0;
    for (const auto &[weight, value, count] : items)
    {
        if (weight == 0)
        {
            zero_weight = KnapsackAdd(zero_weight, value, count);
        }
    }
    std::vector<std::int64_t> best(capacity + 1, exact ? -1 : zero_weight);
    best[0] = zero_weight;
    struct Candidate
    {
        std::size_t step;
        std::int64_t base;
    };
    std::vector<Candidate> queue;
    for (const auto &[weight, value, count] : items)
    {
        if (count == 0 || weight == 0 || weight > capacity)
        {
            continue;
        }
        const auto limit = capacity / weight;
        if (count == 1)
        {
            for (std::size_t i = capacity + 1; i > weight;)
            {
                --i;
                if (best[i - weight] >= 0)
                {
                    best[i] = std::max(best[i], KnapsackAdd(best[i - weight], value, 1));
                }
            }
            continue;
        }
        if (count >= limit)
        {
            for (std::size_t i = weight; i <= capacity; ++i)
            {
                if (best[i - weight] >= 0)
                {
                    best[i] = std::max(best[i], KnapsackAdd(best[i - weight], value, 1));
                }
            }
            continue;
        }

        // 重さの剰余が同じ位置だけを回る。候補は交差しないので、後から並ぶ弱い候補を除ける。
        queue.resize(limit + 1);
        for (std::size_t residue = 0; residue < weight; ++residue)
        {
            const auto last = (capacity - residue) / weight;
            std::size_t begin = 0, end = 0;
            for (std::size_t step = 0; step <= last; ++step)
            {
                const auto i = residue + step * weight;
                if (begin < end && step - queue[begin].step > count)
                {
                    ++begin;
                }
                if (best[i] >= 0)
                {
                    while (begin < end)
                    {
                        const auto &back = queue[end - 1];
                        if (KnapsackAdd(back.base, value, step - back.step) > best[i])
                        {
                            break;
                        }
                        --end;
                    }
                    queue[end++] = {step, best[i]};
                }
                if (begin < end)
                {
                    best[i] = KnapsackAdd(queue[begin].base, value, step - queue[begin].step);
                }
            }
        }
    }
    if (best[capacity] < 0)
    {
        return std::nullopt;
    }
    return best[capacity];
}
}

inline std::int64_t maximize_knapsack(std::size_t capacity, std::span<const KnapsackItem> items)
{
    return *detail::SolveKnapsack(capacity, items, false);
}

inline std::optional<std::int64_t> maximize_exact_knapsack(std::size_t target, std::span<const KnapsackItem> items)
{
    return detail::SolveKnapsack(target, items, true);
}
#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
