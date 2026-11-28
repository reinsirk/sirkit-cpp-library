// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Sparse_Knapsack.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sparse_knapsack.md
// 参考（Verify問題）: https://yukicoder.me/problems/no/626
//
// API一覧
// [SparseKnapsack::Item] weight,value: 非負uint64_t。各品物を1回まで選ぶ。
// SparseKnapsack(capacity,items): 容量以下の支配されない候補を前計算する。
// n品、処理iの直前にP_i候補ならO(n log(n+1)+ΣP_i)時間、O(n+max P_i)領域。実行時間は候補数に依存する。
// max_value([budget]): budget以下で可能な最大価値。既定はcapacity、budget<=capacity。O(log(P+1))。
// frontier(): 価値が正の候補をstd::span<const Item>で返す。重さ・価値が両方昇順。O(1)。
// 最終最適値がuint64_tを超えるなら構築時にoverflow_error。何も選ばない価値は0。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SPARSE_KNAPSACK_DEFINED
#define SIRKIT_SPARSE_KNAPSACK_DEFINED
class SparseKnapsack
{
public:
    struct Item
    {
        std::uint64_t weight = 0, value = 0;
    };

    SparseKnapsack(std::uint64_t capacity, std::span<const Item> items) : capacity_(capacity)
    {
        frontier_.push_back({0, 0});
        std::vector<Item> ordered;
        ordered.reserve(items.size());
        for (const auto &item : items)
        {
            if (item.weight <= capacity && item.value != 0)
            {
                ordered.push_back(item);
            }
        }
        std::sort(ordered.begin(), ordered.end(), [](const Item &a, const Item &b)
                  {
                      return a.weight > b.weight;
                  });
        std::vector<Item> buffer;
        for (const auto &item : ordered)
        {
            Extend(item, buffer);
        }
        if (frontier_.front().value == 0)
        {
            frontier_.erase(frontier_.begin());
        }
    }

    [[nodiscard]] std::uint64_t max_value(std::uint64_t budget) const
    {
        const auto it = UpperWeight(budget);
        return it == frontier_.begin() ? 0 : std::prev(it)->value;
    }

    [[nodiscard]] std::uint64_t max_value() const
    {
        return max_value(capacity_);
    }

    [[nodiscard]] std::span<const Item> frontier() const noexcept
    {
        return frontier_;
    }

private:
    std::uint64_t capacity_;
    std::vector<Item> frontier_;

    std::vector<Item>::const_iterator UpperWeight(std::uint64_t weight) const noexcept
    {
        return std::upper_bound(frontier_.begin(), frontier_.end(), weight,
                                [](std::uint64_t bound, const Item &item)
                                {
                                    return bound < item.weight;
                                });
    }

    void Extend(const Item &item, std::vector<Item> &buffer)
    {
        const auto limit = static_cast<std::size_t>(UpperWeight(capacity_ - item.weight) - frontier_.begin());
        if (item.value > std::numeric_limits<std::uint64_t>::max() - frontier_[limit - 1].value)
        {
            throw std::overflow_error("SparseKnapsack: feasible value exceeds uint64_t");
        }
        buffer.clear();
        const auto append = [&buffer](Item point)
        {
            if (buffer.empty())
            {
                buffer.push_back(point);
            }
            else if (point.value > buffer.back().value)
            {
                if (point.weight == buffer.back().weight)
                {
                    buffer.back().value = point.value;
                }
                else
                {
                    buffer.push_back(point);
                }
            }
        };
        const auto shifted = [&](std::size_t index)
        {
            return Item{frontier_[index].weight + item.weight, frontier_[index].value + item.value};
        };
        // 品物を選ぶ列と選ばない列を重さ順で併合し、重くて価値も低い点を落とす。
        std::size_t a = 0, b = 0;
        while (a < frontier_.size() && b < limit)
        {
            const auto choice = shifted(b);
            if (frontier_[a].weight < choice.weight)
            {
                append(frontier_[a++]);
            }
            else
            {
                append(choice);
                ++b;
            }
        }
        for (; a < frontier_.size(); ++a)
        {
            append(frontier_[a]);
        }
        for (; b < limit; ++b)
        {
            append(shifted(b));
        }
        frontier_.swap(buffer);
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
