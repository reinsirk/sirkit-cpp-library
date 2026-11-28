// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Algorithm/Pair_Product_Order.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pair_product_order.md
//
// API一覧
// PairProductOrder(values): i<jの積values[i]*values[j]を対象にする。
// PairProductOrder(left,right): 二つの列からそれぞれ一つ選ぶ組の積を対象にする。重複する値も添字で区別する。
// size(): 総組数をO(1)で返す。count_at_most(bound): 数学的な積がbound以下の組数をO(n+m+1)で返す。
// kth_smallest(rank): 1始まりのrank番目の積を返す。範囲外の順位ならnullopt。答えがint64_tで表せなければoverflow_error。
// 入力は符号付き64ビット整数、各列長はUINT32_MAX以下。初期化O((n+m+1)log(n+m+2))時間・O(n+m+1)領域。
// 順位検索は最大64回の二分探索。各クエリの追加領域はO(1)。
// コピーは独立。移動元は空として使える。


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
#ifndef SIRKIT_PAIR_PRODUCT_ORDER_DEFINED
#define SIRKIT_PAIR_PRODUCT_ORDER_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void check_pair_product_length(std::uint64_t length)
{
    if (length > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::length_error("PairProductOrder: input is too long");
    }
}

inline constexpr std::uint64_t product_magnitude(std::int64_t negative) noexcept
{
    return std::uint64_t{0} - static_cast<std::uint64_t>(negative);
}

struct ProductMagnitudes
{
    std::vector<std::uint64_t> positive, negative;

    ProductMagnitudes() = default;

    explicit ProductMagnitudes(std::span<const std::int64_t> values)
    {
        check_pair_product_length(values.size());
        for (const std::int64_t value : values)
        {
            if (value > 0)
            {
                positive.push_back(static_cast<std::uint64_t>(value));
            }
            else if (value < 0)
            {
                negative.push_back(product_magnitude(value));
            }
        }
        std::sort(positive.begin(), positive.end());
        std::sort(negative.begin(), negative.end());
    }

    [[nodiscard]] std::uint64_t nonzero_count() const noexcept
    {
        return positive.size() + negative.size();
    }
};

inline constexpr std::uint64_t product_choose_two(std::uint64_t n) noexcept
{
    return n < 2 ? 0 : n * (n - 1) / 2;
}

inline std::uint64_t count_magnitude_products(std::span<const std::uint64_t> rows,
                                              std::span<const std::uint64_t> columns,
                                              std::uint64_t bound) noexcept
{
    if (rows.size() > columns.size())
    {
        std::swap(rows, columns);
    }
    std::uint64_t result = 0;
    auto end = columns.size();
    for (std::size_t index = 0; index < rows.size() && end != 0; ++index)
    {
        const auto limit = bound / rows[index];
        while (end != 0 && columns[end - 1] > limit)
        {
            --end;
        }
        result += end;
    }
    return result;
}

inline std::uint64_t count_distinct_magnitude_products(std::span<const std::uint64_t> magnitudes,
                                                       std::uint64_t bound) noexcept
{
    std::uint64_t result = 0;
    auto end = magnitudes.size();
    for (std::size_t next = 1; next < end; ++next)
    {
        const auto limit = bound / magnitudes[next - 1];
        while (next < end && magnitudes[end - 1] > limit)
        {
            --end;
        }
        result += end - next;
    }
    return result;
}
} // namespace reinsirk::detail

class PairProductOrder
{
public:
    explicit PairProductOrder(std::span<const std::int64_t> values)
        : left_(values), distinct_(true), pair_count_(detail::product_choose_two(values.size())),
          negative_pairs_(static_cast<std::uint64_t>(left_.negative.size()) * left_.positive.size()),
          zero_pairs_(pair_count_ - detail::product_choose_two(left_.nonzero_count()))
    {
    }

    PairProductOrder(std::span<const std::int64_t> left, std::span<const std::int64_t> right)
        : left_(left), right_(right), distinct_(false), pair_count_(static_cast<std::uint64_t>(left.size()) * right.size()),
          negative_pairs_(static_cast<std::uint64_t>(left_.negative.size()) * right_.positive.size() +
                          static_cast<std::uint64_t>(left_.positive.size()) * right_.negative.size()),
          zero_pairs_(pair_count_ - left_.nonzero_count() * right_.nonzero_count())
    {
    }

    PairProductOrder(const PairProductOrder &) = default;

    PairProductOrder(PairProductOrder &&other) noexcept
    {
        swap_state(other);
    }

    PairProductOrder &operator=(PairProductOrder other) noexcept
    {
        swap_state(other);
        return *this;
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        return pair_count_;
    }

    [[nodiscard]] std::uint64_t count_at_most(std::int64_t bound) const noexcept
    {
        if (bound < 0)
        {
            return negative_pairs_ - negative_magnitudes_at_most(detail::product_magnitude(bound) - 1);
        }
        const auto limit = static_cast<std::uint64_t>(bound);
        const auto zero_or_negative = zero_pairs_ + negative_pairs_;
        if (distinct_)
        {
            return zero_or_negative + detail::count_distinct_magnitude_products(left_.positive, limit) +
                   detail::count_distinct_magnitude_products(left_.negative, limit);
        }
        return zero_or_negative + detail::count_magnitude_products(left_.positive, right_.positive, limit) +
               detail::count_magnitude_products(left_.negative, right_.negative, limit);
    }

    [[nodiscard]] std::optional<std::int64_t> kth_smallest(std::uint64_t rank) const
    {
        if (rank == 0 || rank > size())
        {
            return std::nullopt;
        }
        constexpr auto minimum = std::numeric_limits<std::int64_t>::min();
        constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
        auto low = rank <= negative_pairs_ ? minimum : std::int64_t{0};
        auto high = rank <= negative_pairs_ ? std::int64_t{-1} : maximum;
        while (low < high)
        {
            const auto middle = std::midpoint(low, high);
            if (count_at_most(middle) < rank)
            {
                low = middle + 1;
            }
            else
            {
                high = middle;
            }
        }
        if (low == minimum && rank <= negative_pairs_ - negative_magnitudes_at_most(detail::product_magnitude(minimum)))
        {
            throw std::overflow_error("PairProductOrder: product below int64_t range");
        }
        if (low == maximum && rank > count_at_most(maximum))
        {
            throw std::overflow_error("PairProductOrder: product above int64_t range");
        }
        return low;
    }

private:
    void swap_state(PairProductOrder &other) noexcept
    {
        std::swap(left_, other.left_);
        std::swap(right_, other.right_);
        std::swap(distinct_, other.distinct_);
        std::swap(pair_count_, other.pair_count_);
        std::swap(negative_pairs_, other.negative_pairs_);
        std::swap(zero_pairs_, other.zero_pairs_);
    }

    [[nodiscard]] std::uint64_t negative_magnitudes_at_most(std::uint64_t limit) const noexcept
    {
        if (distinct_)
        {
            return detail::count_magnitude_products(left_.negative, left_.positive, limit);
        }
        return detail::count_magnitude_products(left_.negative, right_.positive, limit) +
               detail::count_magnitude_products(left_.positive, right_.negative, limit);
    }

    detail::ProductMagnitudes left_, right_;
    bool distinct_ = false;
    std::uint64_t pair_count_ = 0, negative_pairs_ = 0, zero_pairs_ = 0;
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
