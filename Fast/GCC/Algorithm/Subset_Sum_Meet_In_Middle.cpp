// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Subset_Sum_Meet_In_Middle.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subset_sum_meet_in_middle.md
//
// API一覧
// MeetInMiddleSubsetSum(weights): 44個以下の符号付き重みを前処理する。空集合も数える。
// 正の重みの総和と負の重みの総和は、それぞれint64_tで表現できること。
// select_max_at_most(limit): limit以下で最大の和と、選んだ元の添字を表す64ビットマスクを返す。該当なしならnullopt。
// count_at_most(limit), count_exact(target): 対応する部分集合の個数を返す。同じ重みでも添字が異なれば別に数える。
// nを重みの個数として、前処理・各照会はO(2^ceil(n/2))時間。保存領域はO(2^ceil(n/2))、照会の追加領域はO(1)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBSET_SUM_MEET_IN_MIDDLE_DEFINED
#define SIRKIT_SUBSET_SUM_MEET_IN_MIDDLE_DEFINED

class MeetInMiddleSubsetSum
{
public:
    struct Selection
    {
        std::int64_t sum;
        std::uint64_t mask;
    };

    explicit MeetInMiddleSubsetSum(std::span<const std::int64_t> weights)
        : data_(std::make_shared<const Data>(prepare(weights)))
    {
    }

    MeetInMiddleSubsetSum(const MeetInMiddleSubsetSum &) noexcept = default;
    MeetInMiddleSubsetSum &operator=(const MeetInMiddleSubsetSum &) noexcept = default;

    [[nodiscard]] std::optional<Selection> select_max_at_most(std::int64_t limit) const
    {
        auto last = data_->right.size();
        std::optional<Selection> best;
        for (const auto &left : data_->left)
        {
            last = right_prefix_end<false>(left.sum, limit, last);
            if (last == 0)
            {
                break;
            }
            const auto &right = data_->right[last - 1];
            const auto sum = left.sum + right.sum;
            const auto mask = left.mask | (std::uint64_t{right.mask} << data_->left_size);
            if (sum == limit)
            {
                return Selection{sum, mask};
            }
            if (!best || sum > best->sum)
            {
                best = Selection{sum, mask};
            }
        }
        return best;
    }

    [[nodiscard]] std::uint64_t count_at_most(std::int64_t limit) const
    {
        return count_bounded<false>(limit);
    }

    [[nodiscard]] std::uint64_t count_exact(std::int64_t target) const
    {
        return count_bounded<false>(target) - count_bounded<true>(target);
    }

private:
    struct Entry
    {
        std::int64_t sum;
        std::uint32_t mask;
    };

    struct Data
    {
        std::vector<Entry> left;
        std::vector<Entry> right;
        std::size_t left_size;
    };

    static constexpr std::size_t kMaxSize = 44;
    std::shared_ptr<const Data> data_;

    static Data prepare(std::span<const std::int64_t> weights)
    {
        if (weights.size() > kMaxSize)
        {
            throw std::length_error("MeetInMiddleSubsetSum: too many weights");
        }
        std::int64_t positive = 0;
        std::int64_t negative = 0;
        for (const auto weight : weights)
        {
            if (weight >= 0)
            {
                if (weight > std::numeric_limits<std::int64_t>::max() - positive)
                {
                    throw std::overflow_error("MeetInMiddleSubsetSum: positive sum overflows");
                }
                positive += weight;
            }
            else
            {
                if (weight < std::numeric_limits<std::int64_t>::min() - negative)
                {
                    throw std::overflow_error("MeetInMiddleSubsetSum: negative sum overflows");
                }
                negative += weight;
            }
        }
        const auto left_size = weights.size() / 2;
        return {make_half(weights.first(left_size)), make_half(weights.subspan(left_size)), left_size};
    }

    static std::vector<Entry> make_half(std::span<const std::int64_t> weights)
    {
        std::vector<Entry> entries;
        entries.reserve(std::size_t{1} << weights.size());
        entries.push_back({0, 0});
        for (std::size_t i = 0; i < weights.size(); ++i)
        {
            const auto weight = weights[i];
            const auto bit = std::uint32_t{1} << i;
            std::size_t unselected = entries.size();
            std::size_t selected = entries.size();
            entries.resize(unselected + selected);
            // 整列済みの列を、重みを加えた同じ列と末尾から併合する。
            while (unselected != 0 && selected != 0)
            {
                const auto selected_sum = entries[selected - 1].sum + weight;
                const auto dest = unselected + selected - 1;
                if (entries[unselected - 1].sum > selected_sum)
                {
                    --unselected;
                    entries[dest] = entries[unselected];
                }
                else
                {
                    --selected;
                    entries[dest] = {selected_sum, entries[selected].mask | bit};
                }
            }
            while (selected != 0)
            {
                --selected;
                auto &entry = entries[selected];
                entry.sum += weight;
                entry.mask |= bit;
            }
        }
        return entries;
    }

    template<bool Strict>
    std::size_t right_prefix_end(std::int64_t left_sum, std::int64_t limit, std::size_t last) const
    {
        while (last != 0)
        {
            const auto sum = left_sum + data_->right[last - 1].sum;
            if constexpr (Strict)
            {
                if (sum < limit)
                {
                    break;
                }
            }
            else if (sum <= limit)
            {
                break;
            }
            --last;
        }
        return last;
    }

    template<bool Strict>
    std::uint64_t count_bounded(std::int64_t limit) const
    {
        std::uint64_t count = 0;
        auto last = data_->right.size();
        for (const auto &left : data_->left)
        {
            last = right_prefix_end<Strict>(left.sum, limit, last);
            if (last == 0)
            {
                break;
            }
            count += last;
        }
        return count;
    }
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
