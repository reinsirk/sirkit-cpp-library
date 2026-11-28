// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Pair_Sum_Order.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pair_sum_order.md
//
// API一覧
// PairSumOrder(values): 1列でi<jとなる組の和を対象にする。
// PairSumOrder(left,right): 2列の全ての組の和を対象にする。同値も添字で区別する。
// 各列はint64_t、長さはUINT32_MAX以下。整列してコピーする。O((n+m+1)log(n+m+2))時間、O(n+m+1)領域。
// size(): 組数。sorted_left()/sorted_right(): 昇順の入力列の参照。各O(1)。
// count_below(x), count_at_most(x), count_equal(x): x未満、以下、等しい和の個数。各O(n+m+1)。
// kth_smallest(rank)/kth_largest(rank): 1始まりの順位の和。順位範囲外はnullopt。
// 各順位検索は最大64回の二分探索と範囲検査。和の符号付き64ビット範囲外を選ぶとoverflow_error。
// 個数照会は数学的な和で比較する。コピーは独立、ムーブ元は空。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PAIR_SUM_ORDER_DEFINED
#define SIRKIT_PAIR_SUM_ORDER_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline constexpr auto kMinimumPairSum = std::numeric_limits<std::int64_t>::min();
inline constexpr auto kMaximumPairSum = std::numeric_limits<std::int64_t>::max();

inline void ValidatePairSumLength(std::uint64_t length)
{
    if (length > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::length_error("PairSumOrder: input is too long");
    }
}

inline auto CopyAndSortPairSumInput(std::span<const std::int64_t> values)
{
    ValidatePairSumLength(values.size());
    std::vector<std::int64_t> result(values.begin(), values.end());
    std::sort(result.begin(), result.end());
    return result;
}

[[nodiscard]] inline std::int64_t ClampPairSum(std::int64_t a, std::int64_t b) noexcept
{
    if (a > 0 && b > kMaximumPairSum - a)
    {
        return kMaximumPairSum;
    }
    if (a < 0 && b < kMinimumPairSum - a)
    {
        return kMinimumPairSum;
    }
    return a + b;
}

template <bool Inclusive>
[[nodiscard]] std::size_t PairSumPrefixEnd(std::span<const std::int64_t> sorted,
                                           std::size_t begin, std::size_t end,
                                           std::int64_t a, std::int64_t bound) noexcept
{
    if (a > 0 && bound < kMinimumPairSum + a)
    {
        return begin;
    }
    if (a < 0 && bound > kMaximumPairSum + a)
    {
        return end;
    }
    const auto limit = bound - a;
    const auto outside = [limit](std::int64_t value) {
        if constexpr (Inclusive)
        {
            return value > limit;
        }
        else
        {
            return value >= limit;
        }
    };
    while (end > begin && outside(sorted[end - 1]))
    {
        --end;
    }
    return end;
}
} // namespace reinsirk::detail

class PairSumOrder
{
    std::vector<std::int64_t> left_, right_;
    bool within_;

    template <bool Inclusive>
    [[nodiscard]] std::uint64_t count(std::int64_t bound) const noexcept
    {
        const auto columns = sorted_right();
        auto end = columns.size();
        std::uint64_t answer = 0;
        for (std::size_t i = 0; i < left_.size(); ++i)
        {
            const auto begin = within_ ? i + 1 : 0;
            if (begin >= end)
            {
                break;
            }
            end = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::PairSumPrefixEnd<Inclusive>(columns, begin, end, left_[i], bound);
            answer += end - begin;
        }
        return answer;
    }

public:
    explicit PairSumOrder(std::span<const std::int64_t> values)
        : left_(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CopyAndSortPairSumInput(values)), within_(true)
    {
    }

    PairSumOrder(std::span<const std::int64_t> left, std::span<const std::int64_t> right)
        : left_(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CopyAndSortPairSumInput(left)),
          right_(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CopyAndSortPairSumInput(right)), within_(false)
    {
    }

    PairSumOrder(const PairSumOrder &) = default;

    PairSumOrder(PairSumOrder &&other) noexcept
        : left_(std::move(other.left_)), right_(std::move(other.right_)), within_(other.within_)
    {
        other.left_.clear();
        other.right_.clear();
    }

    PairSumOrder &operator=(PairSumOrder other) noexcept
    {
        left_.swap(other.left_);
        right_.swap(other.right_);
        std::swap(within_, other.within_);
        return *this;
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        const auto n = static_cast<std::uint64_t>(left_.size());
        if (within_)
        {
            return n < 2 ? 0 : n * (n - 1) / 2;
        }
        return n * right_.size();
    }

    [[nodiscard]] std::span<const std::int64_t> sorted_left() const noexcept
    {
        return left_;
    }

    [[nodiscard]] std::span<const std::int64_t> sorted_right() const noexcept
    {
        if (within_)
        {
            return left_;
        }
        return right_;
    }

    [[nodiscard]] std::uint64_t count_below(std::int64_t bound) const noexcept
    {
        return count<false>(bound);
    }

    [[nodiscard]] std::uint64_t count_at_most(std::int64_t bound) const noexcept
    {
        return count<true>(bound);
    }

    [[nodiscard]] std::uint64_t count_equal(std::int64_t target) const noexcept
    {
        return count_at_most(target) - count_below(target);
    }

    [[nodiscard]] std::optional<std::int64_t> kth_smallest(std::uint64_t rank) const
    {
        if (rank == 0 || rank > size())
        {
            return std::nullopt;
        }
        const auto columns = sorted_right();
        const auto first_column = within_ ? 1 : 0;
        const auto last_row = left_.size() - (within_ ? 2 : 1);
        auto left = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ClampPairSum(left_.front(), columns[first_column]);
        auto right = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ClampPairSum(left_[last_row], columns.back());
        if ((left == /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::kMinimumPairSum && count_below(left) >= rank) ||
            (right == /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::kMaximumPairSum && count_at_most(right) < rank))
        {
            throw std::overflow_error("PairSumOrder: sum does not fit in int64_t");
        }
        while (left < right)
        {
            const auto mid = std::midpoint(left, right);
            if (count_at_most(mid) < rank)
            {
                left = mid + 1;
            }
            else
            {
                right = mid;
            }
        }
        return left;
    }

    [[nodiscard]] std::optional<std::int64_t> kth_largest(std::uint64_t rank) const
    {
        const auto total = size();
        if (rank == 0 || rank > total)
        {
            return std::nullopt;
        }
        return kth_smallest(total - rank + 1);
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
