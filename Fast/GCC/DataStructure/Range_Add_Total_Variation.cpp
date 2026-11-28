// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Range_Add_Total_Variation.cpp
// 参考（隣接差の絶対値和）: https://atcoder.jp/contests/typical90/tasks/typical90_bl
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_add_total_variation.md
//
// API一覧
// RangeAddTotalVariation(values={}): int64_t列から構築。O(n+1)時間・領域。
// range_add(left,right,delta): 0始まりの半開区間にint64_tのdeltaを加える。O(1)。
// total_variation() const: 隣り合う値の差の絶対値の合計をuint64_tで返す。O(1)。
// size() const: 列長。O(1)。コピーは独立、move元の列長は0。
// 初期化時と各加算後に隣接差の絶対値和がuint64_tに収まること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_ADD_TOTAL_VARIATION_DEFINED
#define SIRKIT_RANGE_ADD_TOTAL_VARIATION_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
struct VariationDifference
{
    std::uint64_t magnitude = 0;
    bool negative = false;
};

inline std::uint64_t AddVariationMagnitude(std::uint64_t a, std::uint64_t b)
{
    return a + b;
}

inline VariationDifference VariationBetween(std::int64_t left, std::int64_t right) noexcept
{
    const auto a = static_cast<std::uint64_t>(left);
    const auto b = static_cast<std::uint64_t>(right);
    return left <= right ? VariationDifference{b - a, false} : VariationDifference{a - b, true};
}

inline VariationDifference ShiftVariationDifference(VariationDifference before, VariationDifference step)
{
    if (before.negative == step.negative)
    {
        return {AddVariationMagnitude(before.magnitude, step.magnitude), before.negative};
    }
    if (before.magnitude < step.magnitude)
    {
        return {step.magnitude - before.magnitude, step.negative};
    }
    return {before.magnitude - step.magnitude, before.negative};
}
} // namespace reinsirk::detail

class RangeAddTotalVariation
{
    using Difference = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::VariationDifference;

    std::vector<Difference> differences_;
    std::size_t size_ = 0;
    std::uint64_t variation_ = 0;

public:
    explicit RangeAddTotalVariation(std::span<const std::int64_t> values = {}) : size_(values.size())
    {
        differences_.reserve(size_ == 0 ? 0 : size_ - 1);
        for (std::size_t i = 1; i < size_; ++i)
        {
            const auto difference = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::VariationBetween(values[i - 1], values[i]);
            variation_ = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::AddVariationMagnitude(variation_, difference.magnitude);
            differences_.push_back(difference);
        }
    }

    RangeAddTotalVariation(const RangeAddTotalVariation &) = default;

    RangeAddTotalVariation(RangeAddTotalVariation &&other) noexcept
        : differences_(std::move(other.differences_)), size_(std::exchange(other.size_, 0)),
          variation_(std::exchange(other.variation_, 0))
    {
    }

    RangeAddTotalVariation &operator=(RangeAddTotalVariation other) noexcept
    {
        differences_.swap(other.differences_);
        std::swap(size_, other.size_);
        std::swap(variation_, other.variation_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] std::uint64_t total_variation() const noexcept
    {
        return variation_;
    }

    void range_add(std::size_t left, std::size_t right, std::int64_t delta)
    {
        if (left == right || delta == 0)
        {
            return;
        }
        const auto step = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::VariationBetween(0, delta);
        const bool has_left = left != 0;
        const bool has_right = right != size_;
        const Difference old_left = has_left ? differences_[left - 1] : Difference{};
        const Difference old_right = has_right ? differences_[right - 1] : Difference{};
        const Difference next_left = has_left ? /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ShiftVariationDifference(old_left, step) : Difference{};
        const Difference next_right = has_right
            ? /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ShiftVariationDifference(old_right, {step.magnitude, !step.negative}) : Difference{};

        // 変化する両端を計算してから更新する。合計も古い両端を外してから足し直す。
        auto next = variation_ - old_left.magnitude - old_right.magnitude;
        next = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::AddVariationMagnitude(next, next_left.magnitude);
        next = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::AddVariationMagnitude(next, next_right.magnitude);
        if (has_left)
        {
            differences_[left - 1] = next_left;
        }
        if (has_right)
        {
            differences_[right - 1] = next_right;
        }
        variation_ = next;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
