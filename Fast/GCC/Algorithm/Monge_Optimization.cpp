// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Monge_Optimization.cpp
// 参考: https://ei1333.github.io/library/dp/monotone-minima.hpp
// 参考: https://ei1333.github.io/library/dp/smawk.hpp
// 参考: https://info.atcoder.jp/entry/algorithm_lectures/min_plus_convolution
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/monge_optimization.md
//
// API一覧
// monotone_minima(h,w,better): 各行の最左最適列が非減少な行列。O(h+w log(h+1))時間・O(h)領域。
// smawk(h,w,better): 部分行列でも最左最適列が非減少な行列。O(h+w)時間・領域。
// better(i,j,k)は行iでkがjより真に良いときtrue。0始まり。列が0なら各行-1。h,w>=0。
// min_plus_convolution_convex_arbitrary(a,b): aが凸。O(|a|+|b|)時間・領域。
// min_plus_convolution_convex_convex(a,b): a,bが凸。O(|a|+|b|)時間、結果以外O(1)領域。
// 凸は隣接差分が非減少。min-plusの和と大小比較はTで丸めず正確に扱えること。片方が空なら空列。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MONGE_OPTIMIZATION_DEFINED
#define SIRKIT_MONGE_OPTIMIZATION_DEFINED

namespace detail
{
inline void CheckMongeDimensions([[maybe_unused]] int height, [[maybe_unused]] int width)
{
}

template <class Compare>
void SmawkRows(const std::vector<int> &rows, const std::vector<int> &columns, Compare &better, std::vector<int> &answer)
{
    if (rows.empty())
    {
        return;
    }
    std::vector<int> candidates;
    candidates.reserve(std::min(rows.size(), columns.size()));
    for (int column : columns)
    {
        // 現在の列数に対応する行で、右の列に負けた列を除く。
        while (!candidates.empty() && better(rows[candidates.size() - 1], candidates.back(), column))
        {
            candidates.pop_back();
        }
        if (candidates.size() < rows.size())
        {
            candidates.push_back(column);
        }
    }

    std::vector<int> odd;
    odd.reserve(rows.size() / 2);
    for (std::size_t i = 1; i < rows.size(); i += 2)
    {
        odd.push_back(rows[i]);
    }
    SmawkRows(odd, candidates, better, answer);

    std::size_t left = 0, right = 0;
    for (std::size_t i = 0; i < rows.size(); i += 2)
    {
        const int row = rows[i];
        if (i + 1 < rows.size())
        {
            while (candidates[right] != answer[rows[i + 1]])
            {
                ++right;
            }
        }
        else
        {
            right = candidates.size() - 1;
        }
        int best = candidates[left];
        for (std::size_t j = left + 1; j <= right; ++j)
        {
            if (better(row, best, candidates[j]))
            {
                best = candidates[j];
            }
        }
        answer[row] = best;
        left = right;
    }
}

inline int CheckedMinPlusLength(std::size_t n, std::size_t m)
{
    constexpr auto kLimit = static_cast<std::size_t>(std::numeric_limits<int>::max());
    if (n > kLimit || m > kLimit - n + 1)
    {
        throw std::length_error("min-plus convolution: result length");
    }
    return static_cast<int>(n + m - 1);
}
} // namespace detail

template <class Compare>
std::vector<int> monotone_minima(int height, int width, Compare better)
{
    detail::CheckMongeDimensions(height, width);
    std::vector<int> answer(height, -1);
    if (width == 0)
    {
        return answer;
    }
    const auto solve = [&](const auto &self, int top, int bottom, int left, int right) -> void
    {
        if (top == bottom)
        {
            return;
        }
        const int row = std::midpoint(top, bottom);
        int best = left;
        for (int column = left + 1; column <= right; ++column)
        {
            if (better(row, best, column))
            {
                best = column;
            }
        }
        answer[row] = best;
        self(self, top, row, left, best);
        self(self, row + 1, bottom, best, right);
    };
    solve(solve, 0, height, 0, width - 1);
    return answer;
}

template <class Compare>
std::vector<int> smawk(int height, int width, Compare better)
{
    detail::CheckMongeDimensions(height, width);
    std::vector<int> answer(height, -1);
    if (height == 0 || width == 0)
    {
        return answer;
    }
    std::vector<int> rows(height), columns(width);
    std::iota(rows.begin(), rows.end(), 0);
    std::iota(columns.begin(), columns.end(), 0);
    detail::SmawkRows(rows, columns, better, answer);
    return answer;
}

template <class T>
std::vector<T> min_plus_convolution_convex_arbitrary(const std::vector<T> &a, const std::vector<T> &b)
{
    if (a.empty() || b.empty())
    {
        return {};
    }
    const int length = detail::CheckedMinPlusLength(a.size(), b.size());
    const int n = static_cast<int>(a.size()), m = static_cast<int>(b.size());
    const auto category = [n](int index)
    {
        // 有効列、右側の範囲外、左側の範囲外の順。同じ側では有効範囲に近い列が良い。
        if (index < 0)
        {
            return 1;
        }
        return index < n ? 0 : 2;
    };
    const auto indices = smawk(length, m, [&](int row, int current, int candidate)
                               {
                                   const int left_index = row - current, right_index = row - candidate;
                                   const int left_class = category(left_index), right_class = category(right_index);
                                   if (left_class != right_class)
                                   {
                                       return right_class < left_class;
                                   }
                                   if (left_class != 0)
                                   {
                                       return left_class == 2; // SMAWKの比較はcurrent < candidate。
                                   }
                                   return a[right_index] + b[candidate] < a[left_index] + b[current];
                               });
    std::vector<T> result;
    result.reserve(length);
    for (int i = 0; i < length; ++i)
    {
        result.push_back(a[i - indices[i]] + b[indices[i]]);
    }
    return result;
}

template <class T>
std::vector<T> min_plus_convolution_convex_convex(const std::vector<T> &a, const std::vector<T> &b)
{
    if (a.empty() || b.empty())
    {
        return {};
    }
    std::vector<T> result;
    result.reserve(detail::CheckedMinPlusLength(a.size(), b.size()));
    std::size_t i = 0, j = 0;
    result.push_back(a[i] + b[j]);
    while (i + 1 < a.size() && j + 1 < b.size())
    {
        // 差の値を計算せず、候補となる和で次の傾きを比較する。
        if (a[i + 1] + b[j] < a[i] + b[j + 1])
        {
            ++i;
        }
        else
        {
            ++j;
        }
        result.push_back(a[i] + b[j]);
    }
    while (++i < a.size())
    {
        result.push_back(a[i] + b[j]);
    }
    while (++j < b.size())
    {
        result.push_back(a.back() + b[j]);
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
