// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Algorithm/Maximum_Allowed_Pair.cpp
// 参考（問題設定）: https://atcoder.jp/contests/abc331/tasks/abc331_e
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/maximum_allowed_pair.md
//
// API一覧
// MaximumAllowedPairResult: 最大和を取る元の添字 left,right と、その和 sum。
// maximum_allowed_pair(left,right,forbidden): 禁止組を除いた和の最大値と添字を返す。組がなければnullopt。
// 0始まり。同点では(left,right)の辞書順最小。禁止組は重複可。最大和はint64_tで表せること。
// 2列の長さをn,m、禁止指定数をpとする。短い列を候補側とし、kをその長さと「1+行ごとの禁止指定数の最大」の小さい方とする。
// O(n+m+p+min(n,m)*log(k+1))時間、O(n+m+p)領域。空の列を含む場合の時間はO(p+1)。
// 検査有効時は範囲外の禁止添字・最大和の型範囲超過を例外で通知。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MAXIMUM_ALLOWED_PAIR_DEFINED
#define SIRKIT_MAXIMUM_ALLOWED_PAIR_DEFINED

struct MaximumAllowedPairResult
{
    std::size_t left;
    std::size_t right;
    std::int64_t sum;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
// 両項を 2^63 ずらし、和の65ビットを桁上がりと下位64ビットに分ける。
inline std::pair<bool, std::uint64_t> AllowedPairExactSum(std::int64_t left, std::int64_t right) noexcept
{
    constexpr std::uint64_t kSignBit = std::uint64_t{1} << 63;
    const std::uint64_t a = static_cast<std::uint64_t>(left) ^ kSignBit;
    const std::uint64_t b = static_cast<std::uint64_t>(right) ^ kSignBit;
    const std::uint64_t low = a + b;
    return {low < a, low};
}
} // namespace reinsirk::detail

[[nodiscard]] inline std::optional<MaximumAllowedPairResult> maximum_allowed_pair(
    std::span<const std::int64_t> left, std::span<const std::int64_t> right,
    std::span<const std::pair<std::size_t, std::size_t>> forbidden)
{
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &[i, j] : forbidden)
    {
        if (i >= left.size() || j >= right.size())
        {
            throw std::out_of_range("maximum_allowed_pair: forbidden index");
        }
    }
#endif
    if (left.empty() || right.empty())
    {
        return std::nullopt;
    }
    // 短い列だけをスコア順に並べる。元の添字の順序へ戻すときも orient を使う。
    const bool transposed = left.size() < right.size();
    const auto rows = transposed ? right : left;
    const auto columns = transposed ? left : right;
    const auto orient = [transposed](std::size_t first, std::size_t second)
    {
        return transposed ? std::pair{second, first} : std::pair{first, second};
    };

    std::vector<std::size_t> offsets(rows.size() + 1);
    for (const auto &[i, j] : forbidden)
    {
        const auto [row, column] = orient(i, j);
        ++offsets[row + 1];
    }
    const auto most_forbidden = *std::ranges::max_element(offsets);
    std::partial_sum(offsets.begin(), offsets.end(), offsets.begin());
    std::vector<std::size_t> next(offsets);
    std::vector<std::size_t> forbidden_columns(forbidden.size());
    for (const auto &[i, j] : forbidden)
    {
        const auto [row, column] = orient(i, j);
        forbidden_columns[next[row]++] = column;
    }

    // 各行が先頭から読み飛ばす候補数は、その行の禁止指定数以下。
    const auto candidates = most_forbidden < columns.size() ? most_forbidden + 1 : columns.size();
    std::vector<std::size_t> order(columns.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    using Difference = std::vector<std::size_t>::difference_type;
    std::partial_sort(order.begin(), order.begin() + static_cast<Difference>(candidates), order.end(),
                      [columns](std::size_t i, std::size_t j)
                      {
                          if (columns[i] == columns[j])
                          {
                              return i < j;
                          }
                          return columns[i] > columns[j];
                      });
    order.resize(candidates);

    using Pair = std::pair<std::size_t, std::size_t>;
    using ExactSum = std::pair<bool, std::uint64_t>;
    std::optional<std::pair<ExactSum, Pair>> best;
    std::vector<std::size_t> marked(columns.size(), rows.size());
    for (std::size_t row = 0; row < rows.size(); ++row)
    {
        for (std::size_t i = offsets[row]; i < offsets[row + 1]; ++i)
        {
            marked[forbidden_columns[i]] = row;
        }
        for (const auto column : order)
        {
            if (marked[column] == row)
            {
                continue;
            }
            const auto indices = orient(row, column);
            const auto sum = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::AllowedPairExactSum(rows[row], columns[column]);
            if (!best || best->first < sum || (best->first == sum && indices < best->second))
            {
                best.emplace(sum, indices);
            }
            break;
        }
    }

    if (!best)
    {
        return std::nullopt;
    }
    const auto &[sum, indices] = *best;
    const auto [carry, low] = sum;
#ifndef SKIP_RUNTIME_CHECKS
    constexpr std::uint64_t kSignBit = std::uint64_t{1} << 63;
    if (carry == ((low & kSignBit) != 0))
    {
        throw std::overflow_error("maximum_allowed_pair: sum out of range");
    }
#endif
    return MaximumAllowedPairResult{indices.first, indices.second, static_cast<std::int64_t>(low)};
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
