// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Algorithm/Maximum_Subarray.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/maximum_subarray.md
// 参考（Verify問題）: https://atcoder.jp/contests/awc0097/tasks/awc0097_d
//
// API一覧
// maximum_subarray_sum<Sum=int64_t>(values): 非空連続部分列の最大和と、範囲[left,right)を返す。O(n)時間・O(1)領域。
// maximum_submatrix_sum<Sum=int64_t>(rows,cols,value_at): 非空長方形の最大和と範囲[top,bottom)×[left,right)を返す。
// value_at(row,col)は各マスの値。O(HW min(H,W))時間・O(max(H,W))領域。
// 入力が空ならnullopt。Sum{}は0。比較・加算がO(1)で、途中の和もSumに収まること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MAXIMUM_SUBARRAY_DEFINED
#define SIRKIT_MAXIMUM_SUBARRAY_DEFINED
template <class Sum>
struct MaximumSubarraySum
{
    std::size_t left = 0, right = 0;
    Sum sum{};
};

template <class Sum>
struct MaximumSubmatrixSum
{
    std::size_t top = 0, bottom = 0, left = 0, right = 0;
    Sum sum{};
};

template <class Sum = std::int64_t, class Range>
    requires std::ranges::input_range<const Range>
std::optional<MaximumSubarraySum<Sum>> maximum_subarray_sum(const Range &values)
{
    std::optional<MaximumSubarraySum<Sum>> best;
    std::size_t left = 0, right = 0;
    Sum ending{};
    for (const auto &entry : values)
    {
        const auto value = static_cast<Sum>(entry);
        if (ending < Sum{})
        {
            left = right;
            ending = value;
        }
        else
        {
            ending += value;
        }
        ++right;
        if (!best || best->sum < ending)
        {
            best = MaximumSubarraySum<Sum>{left, right, ending};
        }
    }
    return best;
}

template <class Sum = std::int64_t, class ValueAt>
std::optional<MaximumSubmatrixSum<Sum>> maximum_submatrix_sum(std::size_t rows, std::size_t columns,
                                                              const ValueAt &value_at)
{
    if (rows == 0 || columns == 0)
    {
        return std::nullopt;
    }
    const bool transposed = rows > columns;
    const auto short_side = std::min(rows, columns);
    const auto long_side = std::max(rows, columns);
    std::vector<Sum> sums(long_side);
    std::optional<MaximumSubmatrixSum<Sum>> best;
    for (std::size_t top = 0; top < short_side; ++top)
    {
        std::ranges::fill(sums, Sum{});
        for (auto bottom = top; bottom < short_side; ++bottom)
        {
            for (std::size_t column = 0; column < long_side; ++column)
            {
                sums[column] += static_cast<Sum>(transposed ? value_at(column, bottom) : value_at(bottom, column));
            }
            const auto band = *maximum_subarray_sum<Sum>(sums);
            if (!best || best->sum < band.sum)
            {
                if (transposed)
                {
                    best = MaximumSubmatrixSum<Sum>{band.left, band.right, top, bottom + 1, band.sum};
                }
                else
                {
                    best = MaximumSubmatrixSum<Sum>{top, bottom + 1, band.left, band.right, band.sum};
                }
            }
        }
    }
    return best;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
