// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/modular_pair_sum_order.md
//
// API一覧
// ModularPairSumOrder(left,right,modulus): 二列から一つずつ選ぶ組について、和を正のmodulusで割った余りを扱う。
// uint64_tの値を受け取り、各列長はUINT32_MAX以下。重複する値も添字で区別する。
// size(): 総組数。O(1)。count_at_most(bound): 余りがbound以下の組数。O(n+m+1)。
// kth_smallest(rank): 余りを昇順に並べたときの1始まりの順位。範囲外ならnullopt。O((n+m+1)log(modulus+1))。
// 構築O((n+m+1)log(n+m+2))時間・O(n+m+1)領域。各クエリはO(1)追加領域。
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
#ifndef SIRKIT_MODULAR_PAIR_SUM_ORDER_DEFINED
#define SIRKIT_MODULAR_PAIR_SUM_ORDER_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void check_modular_pair_sum_length(std::uint64_t length)
{
    if (length > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::length_error("ModularPairSumOrder: input is too long");
    }
}

inline std::uint64_t checked_modular_pair_sum_modulus(std::uint64_t modulus)
{
    if (modulus == 0)
    {
        throw std::invalid_argument("ModularPairSumOrder: modulus must be positive");
    }
    return modulus;
}

inline std::vector<std::uint64_t> sorted_pair_sum_residues(std::span<const std::uint64_t> values,
                                                            std::uint64_t modulus)
{
    check_modular_pair_sum_length(values.size());
    std::vector<std::uint64_t> residues;
    residues.reserve(values.size());
    for (const auto value : values)
    {
        residues.push_back(value < modulus ? value : value % modulus);
    }
    std::sort(residues.begin(), residues.end());
    return residues;
}
} // namespace reinsirk::detail

class ModularPairSumOrder
{
public:
    ModularPairSumOrder(std::span<const std::uint64_t> left, std::span<const std::uint64_t> right,
                        std::uint64_t modulus)
        : modulus_(detail::checked_modular_pair_sum_modulus(modulus)),
          left_(detail::sorted_pair_sum_residues(left, modulus_)),
          right_(detail::sorted_pair_sum_residues(right, modulus_))
    {
    }

    ModularPairSumOrder(const ModularPairSumOrder &) = default;

    ModularPairSumOrder(ModularPairSumOrder &&other) noexcept
    {
        swap_state(other);
    }

    ModularPairSumOrder &operator=(ModularPairSumOrder other) noexcept
    {
        swap_state(other);
        return *this;
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        return static_cast<std::uint64_t>(left_.size()) * right_.size();
    }

    [[nodiscard]] std::uint64_t count_at_most(std::uint64_t bound) const noexcept
    {
        if (bound >= modulus_ - 1)
        {
            return size();
        }
        std::uint64_t count = 0;
        auto ordinary_end = right_.size();
        auto wrapped_begin = right_.size();
        auto wrapped_end = right_.size();
        for (const auto residue : left_)
        {
            const auto wrap_at = modulus_ - residue;
            while (wrapped_begin != 0 && right_[wrapped_begin - 1] >= wrap_at)
            {
                --wrapped_begin;
            }
            if (residue > bound)
            {
                ordinary_end = 0;
                // b <= modulus + bound - residue を、modulus + bound を作らずに比較する。
                const auto limit = modulus_ - (residue - bound);
                while (wrapped_end > wrapped_begin && right_[wrapped_end - 1] > limit)
                {
                    --wrapped_end;
                }
            }
            else
            {
                const auto limit = bound - residue;
                while (ordinary_end != 0 && right_[ordinary_end - 1] > limit)
                {
                    --ordinary_end;
                }
            }
            count += static_cast<std::uint64_t>(ordinary_end) + (wrapped_end - wrapped_begin);
        }
        return count;
    }

    [[nodiscard]] std::optional<std::uint64_t> kth_smallest(std::uint64_t rank) const noexcept
    {
        if (rank == 0 || rank > size())
        {
            return std::nullopt;
        }
        std::uint64_t low = 0;
        auto high = modulus_ - 1;
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
        return low;
    }

private:
    void swap_state(ModularPairSumOrder &other) noexcept
    {
        std::swap(modulus_, other.modulus_);
        left_.swap(other.left_);
        right_.swap(other.right_);
    }

    std::uint64_t modulus_ = 1;
    std::vector<std::uint64_t> left_, right_;
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
