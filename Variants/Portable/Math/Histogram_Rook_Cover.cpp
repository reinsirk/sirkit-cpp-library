// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Math/Histogram_Rook_Cover.cpp
// 参考（ヒストグラム盤の行を連続区間に分ける見方とVerify問題）: https://img.atcoder.jp/agc041/editorial.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/histogram_rook_cover.md
//
// API一覧
// count_histogram_rook_covers(heights): 全マスがルークに支配される置き方の数を法998244353で返す。
// 列iの下からheights[i]個が盤のマス。支配は同じ列または同じ高さの切れ目のない行区間にルークがあること。
// 隣り合うルークも置ける。heightsは非負のuint64_t列。空盤は1通り。
// n列・最大高さHでO(1+n^2 log(H+2))時間、O(1+n)領域。盤のマスを展開しない。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_HISTOGRAM_ROOK_COVER
#define SIRKIT_HISTOGRAM_ROOK_COVER

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
constexpr std::uint32_t kHistogramRookMod = 998244353;

[[nodiscard]] constexpr std::uint32_t histogram_rook_add(std::uint32_t a, std::uint32_t b)
{
    const auto sum = a + b;
    return sum < kHistogramRookMod ? sum : sum - kHistogramRookMod;
}

[[nodiscard]] constexpr std::uint32_t histogram_rook_subtract(std::uint32_t a, std::uint32_t b)
{
    return histogram_rook_add(a, kHistogramRookMod - b);
}

[[nodiscard]] constexpr std::uint32_t histogram_rook_multiply(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kHistogramRookMod);
}

[[nodiscard]] constexpr std::uint32_t histogram_rook_power(std::uint32_t base, std::uint64_t exponent)
{
    std::uint32_t result = 1;
    while (exponent != 0)
    {
        if ((exponent & 1) != 0)
        {
            result = histogram_rook_multiply(result, base);
        }
        base = histogram_rook_multiply(base, base);
        exponent >>= 1;
    }
    return result;
}

[[nodiscard]] inline std::vector<std::uint32_t> histogram_rook_merge(
    const std::vector<std::uint32_t> &left, const std::vector<std::uint32_t> &right)
{
    std::vector<std::uint32_t> result(left.size() + right.size() - 1);
    for (std::size_t i = 0; i < left.size(); ++i)
    {
        for (std::size_t j = 0; j < right.size(); ++j)
        {
            result[i + j] = histogram_rook_add(result[i + j], histogram_rook_multiply(left[i], right[j]));
        }
    }
    return result;
}

struct HistogramRookState
{
    // 添字はルークを置いてよい列数。空列と仮定した列を含むかどうかで重みを分ける。
    std::vector<std::uint32_t> without_empty;
    std::vector<std::uint32_t> total;
};

[[nodiscard]] inline HistogramRookState histogram_rook_extend(const HistogramRookState &state, std::uint64_t rows)
{
    const auto size = state.total.size();
    HistogramRookState result{std::vector<std::uint32_t>(size + 1), std::vector<std::uint32_t>(size + 1)};
    for (std::size_t i = 0; i < size; ++i)
    {
        // 新しい列を使う、空列とみなす、包除で除く、の3通り。後二者はtotalで相殺する。
        result.without_empty[i] = histogram_rook_subtract(result.without_empty[i], state.without_empty[i]);
        result.without_empty[i + 1] = state.without_empty[i];
        result.total[i + 1] = state.total[i];
    }
    if (rows == 0)
    {
        return result;
    }
    const auto power_of_two = histogram_rook_power(2, rows);
    std::uint32_t subset_count = 1, all_choices = 1;
    for (std::size_t i = 0; i <= size; ++i)
    {
        // 指定した空列が含まれる行では、行が空になる選び方を除く。
        const auto nonempty_choices = histogram_rook_power(histogram_rook_subtract(subset_count, 1), rows);
        const auto with_empty_column = histogram_rook_subtract(result.total[i], result.without_empty[i]);
        result.without_empty[i] = histogram_rook_multiply(result.without_empty[i], all_choices);
        result.total[i] = histogram_rook_add(result.without_empty[i], histogram_rook_multiply(with_empty_column, nonempty_choices));
        all_choices = histogram_rook_multiply(all_choices, power_of_two);
        subset_count = histogram_rook_add(subset_count, subset_count);
    }
    return result;
}
} // namespace reinsirk::detail

[[nodiscard]] inline std::uint32_t count_histogram_rook_covers(std::span<const std::uint64_t> heights)
{
    const auto n = heights.size();
    if (n == 0)
    {
        return 1;
    }
    std::vector<std::array<std::size_t, 2>> children(n, {n, n});
    std::vector<std::size_t> stack;
    stack.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        auto previous = n;
        while (!stack.empty() && heights[stack.back()] > heights[i])
        {
            previous = stack.back();
            stack.pop_back();
        }
        children[i][0] = previous;
        if (!stack.empty())
        {
            children[stack.back()][1] = i;
        }
        stack.push_back(i);
    }
    const auto root = stack.front();
    stack.clear();
    stack.push_back(root);
    std::vector<std::uint64_t> level(n);
    for (std::size_t i = 0; i < stack.size(); ++i)
    {
        const auto at = stack[i];
        for (const auto next : children[at])
        {
            if (next != n)
            {
                level[next] = heights[at];
                stack.push_back(next);
            }
        }
    }
    // 計算済みで、親にまだ統合していない部分木だけを保持する。
    std::vector<detail::HistogramRookState> dp(n);
    for (auto it = stack.rbegin(); it != stack.rend(); ++it)
    {
        const auto at = *it;
        detail::HistogramRookState state{{1}, {1}};
        for (const auto child : children[at])
        {
            if (child != n)
            {
                state.without_empty = detail::histogram_rook_merge(state.without_empty, dp[child].without_empty);
                state.total = detail::histogram_rook_merge(state.total, dp[child].total);
                dp[child] = {};
            }
        }
        dp[at] = detail::histogram_rook_extend(state, heights[at] - level[at]);
    }
    return std::accumulate(dp[root].total.begin(), dp[root].total.end(), std::uint32_t{0}, detail::histogram_rook_add);
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
