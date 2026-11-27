// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Concave_Min_Plus_Convolution.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/concave_min_plus_convolution.md
//
// API一覧
// min_plus_convolution_concave_arbitrary(a,b): kごとにmin(a[i]+b[j]) (i+j=k)を返す。
// aは隣接差分が非増加の凹数列、bは任意。int64_tで和が表現可能であること。片方が空なら空。
// n=a.size()、m=b.size()。結果の長さはINT_MAX以下。O((n+m)log(n+m+1))時間・O(n+m)領域。
//
// [同梱の行最小値探索]
// 参考: https://ei1333.github.io/library/dp/monotone-minima.hpp
// 参考: https://ei1333.github.io/library/dp/smawk.hpp
// 参考: https://info.atcoder.jp/entry/algorithm_lectures/min_plus_convolution
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
#include <algorithm>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <numeric>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <span>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_CONCAVE_MIN_PLUS_CONVOLUTION_DEFINED
#define SIRKIT_CONCAVE_MIN_PLUS_CONVOLUTION_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline bool ConcaveSteps(std::int64_t a, std::int64_t b, std::int64_t c)
{
    const auto left_nonnegative = b >= a;
    const auto right_nonnegative = c >= b;
    if (left_nonnegative != right_nonnegative)
    {
        return left_nonnegative;
    }
    // 符号が同じなら、64ビットの剰余表現でも差分の大小は保たれる。
    const auto left = static_cast<std::uint64_t>(b) - static_cast<std::uint64_t>(a);
    const auto right = static_cast<std::uint64_t>(c) - static_cast<std::uint64_t>(b);
    return left >= right;
}

inline bool ConcaveSumFits(std::int64_t x, std::int64_t y)
{
    if (x >= 0)
    {
        return y <= std::numeric_limits<std::int64_t>::max() - x;
    }
    return y >= std::numeric_limits<std::int64_t>::min() - x;
}
}

inline std::vector<std::int64_t> min_plus_convolution_concave_arbitrary(std::span<const std::int64_t> a,
                                                                        std::span<const std::int64_t> b)
{
    if (a.empty() || b.empty())
    {
        return {};
    }
    const auto length = static_cast<std::size_t>(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedMinPlusLength(a.size(), b.size()));
    const auto [amin, amax] = std::minmax_element(a.begin(), a.end());
    const auto [bmin, bmax] = std::minmax_element(b.begin(), b.end());
    if (!/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ConcaveSumFits(*amin, *bmin) || !/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ConcaveSumFits(*amax, *bmax))
    {
        throw std::overflow_error("min_plus_convolution_concave_arbitrary: sum exceeds int64_t");
    }
    std::vector<std::int64_t> result(length, std::numeric_limits<std::int64_t>::max());
    const auto width = a.size();
    const auto apply = [&](std::size_t left, std::size_t right, std::size_t first, std::size_t last)
    {
        if (first >= last)
        {
            return;
        }
        if (right - left <= 4 || last - first <= 4)
        {
            for (auto k = left; k < right; ++k)
            {
                for (auto j = first; j < last; ++j)
                {
                    result[k] = std::min(result[k], a[k - j] + b[j]);
                }
            }
            return;
        }
        // 凹列に対する最小列は、bの添字を逆順に並べると非減少になる。
        const auto cost = [&](int row, int column)
        {
            const auto k = left + static_cast<std::size_t>(row);
            const auto j = last - 1 - static_cast<std::size_t>(column);
            return a[k - j] + b[j];
        };
        const auto argmin = smawk(static_cast<int>(right - left), static_cast<int>(last - first),
                                  [&](int r, int j, int k)
                                  {
                                      return cost(r, k) < cost(r, j);
                                  });
        for (std::size_t i = 0; i < argmin.size(); ++i)
        {
            result[left + i] = std::min(result[left + i], cost(static_cast<int>(i), argmin[i]));
        }
    };
    const auto solve = [&](const auto &self, std::size_t left, std::size_t right, std::size_t lo, std::size_t hi) -> void
    {
        // b[j]が有効な出力区間は[j,j+width)。親で扱えなかった候補だけをこの区間へ割り当てる。
        const auto lower = right > width ? right - width : 0;
        apply(left, right, std::max(lo, lower), std::min(hi, left + 1));
        if (right - left > 1)
        {
            const auto middle = std::midpoint(left, right);
            self(self, left, middle, 0, lower);
            self(self, middle, right, left + 1, b.size());
        }
    };
    solve(solve, 0, length, 0, b.size());
    return result;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
