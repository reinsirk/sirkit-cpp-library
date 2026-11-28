// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Algorithm/Knapsack_By_Value.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/knapsack_by_value.md
// 参考（Verify問題）: https://atcoder.jp/contests/dp/tasks/dp_e
//
// API一覧
// [KnapsackByValue::Item] weight:uint64_t、value:size_t。各品物は1回まで選べる。0も可。
// KnapsackByValue(items): 価値を添字に最小重量を前計算する。n品、総価値VでO((n+1)(V+1))時間・O(V+1)領域。
// minimum_weight(value): 価値がちょうどvalueとなる最小重量。到達不能またはuint64_t外ならnullopt。O(1)。
// max_value(budget): 重量がbudget以下で選べる最大価値。O(log(V+2))。
// 何も選ばない価値・重量は0。V+1個の表を確保できること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_KNAPSACK_BY_VALUE_DEFINED
#define SIRKIT_KNAPSACK_BY_VALUE_DEFINED
class KnapsackByValue
{
public:
    struct Item
    {
        std::uint64_t weight = 0;
        std::size_t value = 0;
    };

    explicit KnapsackByValue(std::span<const Item> items)
    {
        const auto max_value = min_weight_.max_size() - 1;
        std::size_t total = 0;
        for (const auto &item : items)
        {
            if (item.value > max_value - total)
            {
                throw std::length_error("KnapsackByValue: total value exceeds table capacity");
            }
            total += item.value;
        }
        min_weight_.resize(total + 1);
        min_weight_[0] = 0;
        std::size_t processed = 0;
        for (const auto &[weight, value] : items)
        {
            if (value == 0)
            {
                continue;
            }
            // 価値を降順に更新して、同じ品物の再利用を防ぐ。
            for (auto from = processed + 1; from-- != 0;)
            {
                const auto previous = min_weight_[from];
                if (!previous || weight > std::numeric_limits<std::uint64_t>::max() - *previous)
                {
                    continue;
                }
                const auto chosen = *previous + weight;
                auto &target = min_weight_[from + value];
                if (!target || chosen < *target)
                {
                    target = chosen;
                }
            }
            processed += value;
        }
        // より高い価値が同じか軽い重量で作れる候補を取り除く。
        for (auto value = total; value != 0; --value)
        {
            const auto weight = min_weight_[value];
            if (weight && (frontier_.empty() || *weight < frontier_.back().weight))
            {
                frontier_.push_back({*weight, value});
            }
        }
        std::reverse(frontier_.begin(), frontier_.end());
    }

    [[nodiscard]] std::optional<std::uint64_t> minimum_weight(std::size_t value) const noexcept
    {
        return value < min_weight_.size() ? min_weight_[value] : std::nullopt;
    }

    [[nodiscard]] std::size_t max_value(std::uint64_t budget) const noexcept
    {
        const auto it = std::upper_bound(frontier_.begin(), frontier_.end(), budget,
                                         [](std::uint64_t limit, const Item &item)
                                         {
                                             return limit < item.weight;
                                         });
        return it == frontier_.begin() ? 0 : std::prev(it)->value;
    }

private:
    std::vector<std::optional<std::uint64_t>> min_weight_;
    std::vector<Item> frontier_;
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
