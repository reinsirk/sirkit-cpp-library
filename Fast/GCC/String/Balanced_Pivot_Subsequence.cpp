// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Balanced_Pivot_Subsequence.cpp
// 参考（照会の用途）: https://atcoder.jp/contests/abc381/tasks/abc381_e
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/balanced_pivot_subsequence.md
//
// API一覧
// BalancedPivotSubsequenceIndex(text,left,middle,right): 3バイトを指定し、固定文字列をO(n+1)時間・領域で前処理。
// size(): 元の文字列のバイト数、O(1)。コピーは独立し、移動元は空になる。
// query(l,r): 半開区間[l,r)でleft^k,middle,right^kとなる最長部分列の中央位置とk。中央がなければnullopt。
// queryはO(1+log(n+1))時間。同点なら最小の中央位置を返す。指定3バイトが重なってもよい。
// BalancedPivotSubsequenceResultのpivotは元の0始まり位置、pairsは両側それぞれの個数。length(): 2*pairs+1、O(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BALANCED_PIVOT_SUBSEQUENCE_DEFINED
#define SIRKIT_BALANCED_PIVOT_SUBSEQUENCE_DEFINED

struct BalancedPivotSubsequenceResult
{
    std::size_t pivot;
    std::size_t pairs;

    [[nodiscard]] std::size_t length() const noexcept
    {
        return pairs * 2 + 1;
    }
};

class BalancedPivotSubsequenceIndex
{
    struct Counts
    {
        std::size_t left;
        std::size_t right;
    };

    std::vector<Counts> prefixes_;
    std::vector<std::size_t> pivots_;

public:
    BalancedPivotSubsequenceIndex(std::string_view text, char left, char middle, char right)
        : prefixes_(text.size() + 1)
    {
        for (std::size_t i = 0; i < text.size(); ++i)
        {
            const char byte = text[i];
            prefixes_[i + 1] = {prefixes_[i].left + (byte == left), prefixes_[i].right + (byte == right)};
            if (byte == middle)
            {
                pivots_.push_back(i);
            }
        }
    }

    BalancedPivotSubsequenceIndex(const BalancedPivotSubsequenceIndex &) = default;

    BalancedPivotSubsequenceIndex(BalancedPivotSubsequenceIndex &&other) noexcept
    {
        prefixes_.swap(other.prefixes_);
        pivots_.swap(other.pivots_);
    }

    BalancedPivotSubsequenceIndex &operator=(BalancedPivotSubsequenceIndex other) noexcept
    {
        prefixes_.swap(other.prefixes_);
        pivots_.swap(other.pivots_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return prefixes_.empty() ? 0 : prefixes_.size() - 1;
    }

    [[nodiscard]] std::optional<BalancedPivotSubsequenceResult> query(std::size_t left, std::size_t right) const
    {
        const auto first = std::lower_bound(pivots_.begin(), pivots_.end(), left);
        const auto beyond = std::lower_bound(first, pivots_.end(), right);
        if (first == beyond)
        {
            return std::nullopt;
        }
        const auto left_count = [&](std::size_t pivot)
        {
            return prefixes_[pivot].left - prefixes_[left].left;
        };
        const auto right_count = [&](std::size_t pivot)
        {
            return prefixes_[right].right - prefixes_[pivot + 1].right;
        };

        // 中央を右へ動かすと左側の個数は増加し、右側の個数は減少する。
        const auto split = std::partition_point(first, beyond, [&](std::size_t pivot)
        {
            return left_count(pivot) < right_count(pivot);
        });
        const auto left_pairs = split == first ? 0 : left_count(*std::prev(split));
        const auto right_pairs = split == beyond ? 0 : right_count(*split);
        const auto pairs = std::max(left_pairs, right_pairs);

        // 最適な個数の左側を取れる最初の中央では、右側の個数も足りる。
        const auto earliest = std::lower_bound(first, beyond, pairs, [&](std::size_t pivot, std::size_t count)
        {
            return left_count(pivot) < count;
        });
        return BalancedPivotSubsequenceResult{*earliest, pairs};
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
