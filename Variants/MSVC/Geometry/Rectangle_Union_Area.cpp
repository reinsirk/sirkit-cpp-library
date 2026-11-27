// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Geometry/Rectangle_Union_Area.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/rectangle_union_area.md
//
// API一覧
// AreaRectangle{left,bottom,right,top}: 整数座標で軸に平行な長方形。
// rectangle_union_area(rectangles): 和集合の面積をuint64_tで返す。面積が型を超えるとoverflow_error。
// 幅または高さ0は無視する。座標の上下左右は整列済みとする。O(n log(n+1))時間・O(n)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RECTANGLE_UNION_AREA_DEFINED
#define SIRKIT_RECTANGLE_UNION_AREA_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t rectangle_area_workspace(std::size_t count, std::size_t scale, std::size_t limit)
{
    if (count > limit / scale)
    {
        throw std::length_error("rectangle_union_area: workspace too large");
    }
    return count * scale;
}
} // namespace detail

struct AreaRectangle
{
    std::int64_t left, bottom, right, top;
};

inline std::uint64_t rectangle_union_area(std::span<const AreaRectangle> rectangles)
{
    struct Event
    {
        std::int64_t x, bottom, top;
        bool enter;
    };
    std::vector<Event> events;
    std::vector<std::int64_t> y;
    const auto capacity = detail::rectangle_area_workspace(rectangles.size(), 2, std::min(events.max_size(), y.max_size()));
    events.reserve(capacity);
    y.reserve(capacity);
    for (auto [left, bottom, right, top] : rectangles)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || bottom > top)
        {
            throw std::invalid_argument("rectangle_union_area: reversed interval");
        }
#endif
        if (left == right || bottom == top)
        {
            continue;
        }
        events.push_back({left, bottom, top, true});
        events.push_back({right, bottom, top, false});
        y.push_back(bottom);
        y.push_back(top);
    }
    if (events.empty())
    {
        return 0;
    }

    std::ranges::sort(events, {}, &Event::x);
    std::ranges::sort(y);
    y.erase(std::unique(y.begin(), y.end()), y.end());
    const auto segments = y.size() - 1;
    struct Node
    {
        std::size_t cover = 0;
        std::uint64_t length = 0;
    };
    std::vector<Node> tree;
    tree.resize(detail::rectangle_area_workspace(segments, 4, tree.max_size()));
    const auto length = [](std::int64_t low, std::int64_t high)
    {
        return static_cast<std::uint64_t>(high) - static_cast<std::uint64_t>(low);
    };

    const auto update = [&](auto &&self, std::size_t index, std::size_t left, std::size_t right,
                            std::size_t begin, std::size_t end, bool enter) -> void
    {
        auto &node = tree[index];
        if (begin <= left && right <= end)
        {
            if (enter)
            {
                ++node.cover;
            }
            else
            {
                --node.cover;
            }
        }
        else
        {
            const auto mid = left + (right - left) / 2;
            if (begin < mid)
            {
                self(self, 2 * index, left, mid, begin, end, enter);
            }
            if (mid < end)
            {
                self(self, 2 * index + 1, mid, right, begin, end, enter);
            }
        }
        if (node.cover != 0)
        {
            node.length = length(y[left], y[right]);
        }
        else if (right - left == 1)
        {
            node.length = 0;
        }
        else
        {
            node.length = tree[2 * index].length + tree[2 * index + 1].length;
        }
    };

    auto previous_x = events.front().x;
    std::uint64_t area = 0;
    for (const auto &[x, bottom, top, enter] : events)
    {
        const auto width = length(previous_x, x);
        const auto covered = tree[1].length;
        if (covered != 0 && width > (std::numeric_limits<std::uint64_t>::max() - area) / covered)
        {
            throw std::overflow_error("rectangle_union_area: area overflow");
        }
        area += width * covered;
        previous_x = x;
        const auto begin = static_cast<std::size_t>(std::ranges::lower_bound(y, bottom) - y.begin());
        const auto end = static_cast<std::size_t>(std::ranges::lower_bound(y, top) - y.begin());
        update(update, 1, 0, segments, begin, end, enter);
    }
    return area;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
