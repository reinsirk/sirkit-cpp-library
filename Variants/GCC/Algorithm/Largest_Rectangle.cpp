// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Algorithm/Largest_Rectangle.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/largest_rectangle.md
// 参考（Verify問題）: https://atcoder.jp/contests/abc189/tasks/abc189_c
//
// API一覧
// largest_histogram_rectangle(heights): 幅1の棒に内接する最大長方形。{left,right,height,area}を返す。
// heightsは非負uint64_t列。O(n)時間・O(n)領域。
// largest_grid_rectangle(rows,cols,accepted): 条件を満たすセルだけからなる最大長方形。
// {top,bottom,left,right,area}を返す。accepted(row,col)を行優先で各セル1回呼ぶ。O(HW)時間・O(W)領域。
// 座標は0始まり・半開区間。面積0なら全フィールド0。最大面積がuint64_tを超える場合はoverflow_error。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_LARGEST_RECTANGLE_DEFINED
#define SIRKIT_LARGEST_RECTANGLE_DEFINED
struct HistogramRectangle
{
    std::size_t left = 0, right = 0;
    std::uint64_t height = 0, area = 0;
};

struct GridRectangle
{
    std::size_t top = 0, bottom = 0, left = 0, right = 0;
    std::uint64_t area = 0;
};

namespace detail
{
struct HistogramBar
{
    std::size_t left = 0;
    std::uint64_t height = 0;
};

inline HistogramRectangle ScanHistogram(std::span<const std::uint64_t> heights, std::vector<HistogramBar> &stack)
{
    stack.clear();
    HistogramRectangle best;
    const auto extend = [&](std::size_t right, std::uint64_t height)
    {
        auto left = right;
        // 低い棒が現れたら、それまで延ばせた長方形の右端が決まる。
        while (!stack.empty() && stack.back().height > height)
        {
            const auto bar = stack.back();
            stack.pop_back();
            left = bar.left;
            const auto width = right - left;
            if (width > std::numeric_limits<std::uint64_t>::max() / bar.height)
            {
                throw std::overflow_error("largest_histogram_rectangle: area exceeds uint64_t");
            }
            const auto area = bar.height * width;
            if (area > best.area)
            {
                best = {left, right, bar.height, area};
            }
        }
        if (height != 0 && (stack.empty() || stack.back().height < height))
        {
            stack.push_back({left, height});
        }
    };
    for (std::size_t i = 0; i < heights.size(); ++i)
    {
        extend(i, heights[i]);
    }
    extend(heights.size(), 0);
    return best;
}
}

inline HistogramRectangle largest_histogram_rectangle(std::span<const std::uint64_t> heights)
{
    std::vector<detail::HistogramBar> stack;
    stack.reserve(heights.size());
    return detail::ScanHistogram(heights, stack);
}

template <class Accepted>
GridRectangle largest_grid_rectangle(std::size_t rows, std::size_t columns, Accepted &&accepted)
{
    if (rows == 0 || columns == 0)
    {
        return {};
    }
    std::vector<std::uint64_t> heights(columns);
    std::vector<detail::HistogramBar> stack;
    stack.reserve(columns);
    GridRectangle best;
    // 現在の行を下端として、各列で条件が連続した高さを使う。
    for (std::size_t row = 0; row < rows; ++row)
    {
        for (std::size_t column = 0; column < columns; ++column)
        {
            heights[column] = accepted(row, column) ? heights[column] + 1 : 0;
        }
        const auto current = detail::ScanHistogram(heights, stack);
        if (current.area > best.area)
        {
            best = {row + 1 - static_cast<std::size_t>(current.height), row + 1,
                    current.left, current.right, current.area};
        }
    }
    return best;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
