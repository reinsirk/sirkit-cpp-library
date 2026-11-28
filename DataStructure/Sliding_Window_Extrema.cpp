// SPDX-License-Identifier: CC0-1.0
// 参考: https://atcoder.jp/contests/abc228/editorial/2945
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sliding_window_extrema.md
//
// API一覧
// sliding_window_argextrema(values,k,compare=std::less<>{}): 幅kの各窓で選ぶ要素の0始まり添字を返す。
// sliding_window_extrema(values,k,compare=std::less<>{}): 同じ窓の選んだ値を返す。
// 長さnをO(1)で取得しランダムアクセスできる列。O(n)時間・O(k)作業領域。出力は左端の昇順。
// sliding_window_extrema_2d(grid,kh,kw,compare=std::less<>{}): 高さkh、幅kwの各窓から値を選ぶ。
// gridは長方形のvector<vector<T>>。O(HW)時間・O(H(W-kw+1)+max(kh,kw))追加領域。
// compareはconst呼出し可能な狭義弱順序。std::greater<>{}なら最大値。等価な候補では行、次いで列の小さいものを選ぶ。
// 幅または高さ0、または窓が入力より大きければ空配列。検査有効時に不揃いな行は例外。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SLIDING_WINDOW_EXTREMA_DEFINED
#define SIRKIT_SLIDING_WINDOW_EXTREMA_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
template <class Sequence>
concept WindowExtremaRange =
    std::ranges::random_access_range<const Sequence> && std::ranges::sized_range<const Sequence>;

template <class At, class Compare, class Emit>
void ScanWindowExtrema(std::size_t length, std::size_t width, std::vector<std::size_t> &queue, const At &at,
                       const Compare &compare, const Emit &emit)
{
    std::size_t head = 0;
    std::size_t tail = 0;
    std::size_t count = 0;
    for (std::size_t i = 0; i < length; ++i)
    {
        if (i >= width && queue[head] <= i - width)
        {
            if (++head == width)
            {
                head = 0;
            }
            --count;
        }

        const auto &current = at(i);
        while (count != 0)
        {
            const auto back = (tail == 0 ? width : tail) - 1;
            if (!std::invoke(compare, current, at(queue[back])))
            {
                break;
            }
            tail = back;
            --count;
        }
        queue[tail] = i;
        if (++tail == width)
        {
            tail = 0;
        }
        ++count;
        if (i >= width - 1)
        {
            emit(i - (width - 1), queue[head]);
        }
    }
}

template <bool Indices, WindowExtremaRange Sequence, class Compare>
auto SlidingWindowExtrema(const Sequence &values, std::size_t width, const Compare &compare)
{
    using Element = std::ranges::range_value_t<const Sequence>;
    using Result = std::conditional_t<Indices, std::size_t, Element>;
    std::vector<Result> output;
    const auto length = static_cast<std::size_t>(std::ranges::size(values));
    if (width == 0 || width > length)
    {
        return output;
    }

    std::vector<std::size_t> queue(width);
    output.reserve(length - width + 1);
    const auto at = [first = std::ranges::begin(values)](std::size_t i) -> decltype(auto) {
        return first[static_cast<std::iter_difference_t<decltype(first)>>(i)];
    };
    ScanWindowExtrema(length, width, queue, at, compare, [&](std::size_t, std::size_t selected) {
        if constexpr (Indices)
        {
            output.push_back(selected);
        }
        else
        {
            output.emplace_back(at(selected));
        }
    });
    return output;
}
} // namespace reinsirk::detail

template </*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::WindowExtremaRange Sequence, class Compare = std::less<>>
[[nodiscard]] std::vector<std::size_t> sliding_window_argextrema(const Sequence &values, std::size_t width,
                                                               const Compare &compare = {})
{
    return /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SlidingWindowExtrema<true>(values, width, compare);
}

template </*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::WindowExtremaRange Sequence, class Compare = std::less<>>
[[nodiscard]] std::vector<std::ranges::range_value_t<const Sequence>>
sliding_window_extrema(const Sequence &values, std::size_t width, const Compare &compare = {})
{
    return /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SlidingWindowExtrema<false>(values, width, compare);
}

template <class T, class Compare = std::less<>>
[[nodiscard]] std::vector<std::vector<T>> sliding_window_extrema_2d(
    const std::vector<std::vector<T>> &grid, std::size_t window_height, std::size_t window_width,
    const Compare &compare = {})
{
    const auto height = grid.size();
    const auto width = grid.empty() ? 0 : grid.front().size();
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &row : grid)
    {
        if (row.size() != width)
        {
            throw std::invalid_argument("sliding_window_extrema_2d: ragged grid");
        }
    }
#endif
    if (window_height == 0 || window_width == 0 || window_height > height || window_width > width)
    {
        return {};
    }

    const auto horizontal_width = width - window_width + 1;
    std::vector<std::size_t> queue(std::max(window_height, window_width));
    std::vector<std::vector<T>> output(height);
    for (std::size_t y = 0; y < height; ++y)
    {
        const auto &row = grid[y];
        output[y].reserve(horizontal_width);
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ScanWindowExtrema(
            width, window_width, queue, [&](std::size_t x) -> decltype(auto) { return row[x]; }, compare,
            [&](std::size_t, std::size_t selected) { output[y].push_back(row[selected]); });
    }

    for (std::size_t x = 0; x < horizontal_width; ++x)
    {
        const auto at = [&](std::size_t y) -> decltype(auto) { return std::as_const(output)[y][x]; };
        // 出力を書き込む行は現在の窓の先頭で、次の窓から外れる。
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ScanWindowExtrema(
            height, window_height, queue, at, compare, [&](std::size_t y, std::size_t selected) {
                if (y != selected)
                {
                    output[y][x] = at(selected);
                }
            });
    }
    output.resize(height - window_height + 1);
    return output;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
