// SPDX-License-Identifier: CC0-1.0
// 参考（中央値判定の±1累積和）: https://img.atcoder.jp/arc101/editorial.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subarray_median_order.md
//
// API一覧
// SubarrayMedianSide::upper/lower: 偶数長で大きい側/小さい側の中央値。upperが既定。
// count_subarray_medians_at_least(a,x,side): x以上の中央値を持つ非空連続部分列数。O(n+1)時間・領域。
// kth_subarray_median(a,k,side): 連続部分列の中央値を昇順に並べた1始まりk番目。範囲外はnullopt。
// 順位はO((n+1)log(n+2))時間・O(n+1)領域。部分列は位置で区別。
// aはsize()相当がO(1)の多重走査可能な範囲。n<=UINT32_MAX。要素の順序比較とコピーはO(1)とする。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBARRAY_MEDIAN_ORDER_DEFINED
#define SIRKIT_SUBARRAY_MEDIAN_ORDER_DEFINED

enum class SubarrayMedianSide
{
    upper,
    lower,
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
template <class Sequence>
concept SubarrayMedianRange =
    std::ranges::forward_range<const Sequence> && std::ranges::sized_range<const Sequence> &&
    std::totally_ordered<std::ranges::range_value_t<const Sequence>>;

inline std::size_t SubarrayMedianStorage(std::size_t n, std::size_t maximum)
{
    if (n > std::numeric_limits<std::uint32_t>::max() || maximum == 0 || n > (maximum - 1) / 2)
    {
        throw std::length_error("subarray median: input too long");
    }
    return n * 2 + 1;
}

template <SubarrayMedianRange Sequence>
std::uint64_t CountSubarrayMedianAtLeast(
    const Sequence &values, const std::ranges::range_value_t<const Sequence> &threshold,
    SubarrayMedianSide side, std::vector<std::size_t> &frequency)
{
    std::fill(frequency.begin(), frequency.end(), 0);
    auto balance = frequency.size() / 2;
    frequency[balance] = 1;
    std::uint64_t answer = 0;
    std::size_t at_most = 1;
    for (const auto &value : values)
    {
        // 隣り合う累積和の差は±1なので、現在値以下の個数は1枠の補正で更新できる。
        if (value < threshold)
        {
            at_most -= frequency[balance];
            --balance;
        }
        else
        {
            ++balance;
            at_most += frequency[balance];
        }
        answer += at_most - (side == SubarrayMedianSide::lower ? frequency[balance] : 0);
        ++frequency[balance];
        ++at_most;
    }
    return answer;
}
} // namespace reinsirk::detail

template </*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SubarrayMedianRange Sequence>
[[nodiscard]] std::uint64_t count_subarray_medians_at_least(
    const Sequence &values, const std::ranges::range_value_t<const Sequence> &threshold,
    SubarrayMedianSide side = SubarrayMedianSide::upper)
{
    std::vector<std::size_t> frequency;
    frequency.resize(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SubarrayMedianStorage(std::ranges::size(values), frequency.max_size()));
    return /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CountSubarrayMedianAtLeast(values, threshold, side, frequency);
}

template </*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SubarrayMedianRange Sequence>
[[nodiscard]] std::optional<std::ranges::range_value_t<const Sequence>> kth_subarray_median(
    const Sequence &values, std::uint64_t rank, SubarrayMedianSide side = SubarrayMedianSide::upper)
{
    const auto n = std::ranges::size(values);
    std::vector<std::size_t> frequency;
    const auto capacity = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SubarrayMedianStorage(n, frequency.max_size());
    const auto total = static_cast<std::uint64_t>(n) * (static_cast<std::uint64_t>(n) + 1) / 2;
    if (rank == 0 || rank > total)
    {
        return std::nullopt;
    }

    using Value = std::ranges::range_value_t<const Sequence>;
    std::vector<Value> candidates;
    candidates.reserve(n);
    for (const auto &value : values)
    {
        candidates.push_back(value);
    }
    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

    frequency.resize(capacity);
    const auto wanted = total - rank + 1;
    std::size_t low = 0, high = candidates.size();
    while (low + 1 < high)
    {
        const auto mid = low + (high - low) / 2;
        if (/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CountSubarrayMedianAtLeast(values, candidates[mid], side, frequency) >= wanted)
        {
            low = mid;
        }
        else
        {
            high = mid;
        }
    }
    return candidates[low];
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
