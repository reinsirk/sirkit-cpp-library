// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Geometry/Rectangle_Sum.cpp
// 参考: https://ei1333.github.io/library/other/static-point-add-rectangle-sum.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/rectangle_sum.md
//
// API一覧
// RectangleSumPoint{x,y,weight}: 重み付きの格子点。座標・重みは符号付き64ビット。同じ座標の点も個別に加算。
// RectangleSumQuery{left,bottom,right,top}: 半開長方形[left,right)×[bottom,top)。空なら和は0。
// rectangle_sum(points,queries): 各長方形内の和をクエリ順に返す。O((N+Q) log(N+Q+1))時間・O(N+Q)領域。
// 和は2^64を法とし、符号付き64ビットに対応させて返す。検査有効時、左右または上下が逆ならstd::invalid_argument。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RECTANGLE_SUM_DEFINED
#define SIRKIT_RECTANGLE_SUM_DEFINED

struct RectangleSumPoint
{
    std::int64_t x;
    std::int64_t y;
    std::int64_t weight;
};

struct RectangleSumQuery
{
    std::int64_t left;
    std::int64_t bottom;
    std::int64_t right;
    std::int64_t top;
};

namespace detail
{
inline std::size_t CheckedRectangleSumEventCount(std::size_t queries, std::size_t capacity)
{
    if (queries > capacity / 2)
    {
        throw std::length_error("rectangle_sum: query count");
    }
    return 2 * queries;
}
} // namespace detail

inline std::vector<std::int64_t> rectangle_sum(std::span<const RectangleSumPoint> points,
                                               std::span<const RectangleSumQuery> queries)
{
    if (queries.empty())
    {
        return {};
    }

    auto ordered = std::vector(points.begin(), points.end());
    std::ranges::sort(ordered, {}, &RectangleSumPoint::y);
    std::vector<std::int64_t> ys;
    ys.reserve(ordered.size());
    for (auto &point : ordered)
    {
        if (ys.empty() || ys.back() != point.y)
        {
            ys.push_back(point.y);
        }
        // 元のyはysに保持し、走査時に使う添字をコピー側へ格納する。
        point.y = static_cast<std::int64_t>(ys.size() - 1);
    }

    struct Event
    {
        std::int64_t x;
        std::size_t begin;
        std::size_t end;
        std::size_t id_and_side;
    };
    std::vector<Event> events;
    events.reserve(detail::CheckedRectangleSumEventCount(queries.size(), events.max_size()));
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        const auto &[left, bottom, right, top] = queries[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || bottom > top)
        {
            throw std::invalid_argument("rectangle_sum: reversed rectangle");
        }
#endif
        if (left == right || bottom == top)
        {
            continue;
        }
        const auto lower = std::lower_bound(ys.begin(), ys.end(), bottom);
        const auto upper = std::lower_bound(lower, ys.end(), top);
        if (lower == upper)
        {
            continue;
        }
        const auto begin = static_cast<std::size_t>(lower - ys.begin());
        const auto end = static_cast<std::size_t>(upper - ys.begin());
        events.push_back({left, begin, end, 2 * i});
        events.push_back({right, begin, end, 2 * i + 1});
    }

    std::vector<std::int64_t> answer(queries.size());
    if (events.empty())
    {
        return answer;
    }
    std::ranges::sort(ordered, {}, &RectangleSumPoint::x);
    std::ranges::sort(events, {}, &Event::x);

    // 0始まりのFenwick Tree。符号なしの加減算で途中の和のオーバーフローを避ける。
    std::vector<std::uint64_t> tree(ys.size());
    const auto prefix = [&](std::size_t end)
    {
        std::uint64_t total = 0;
        for (; end != 0; end &= end - 1)
        {
            total += tree[end - 1];
        }
        return total;
    };
    std::size_t next = 0;
    for (const auto &[x, begin, end, id_and_side] : events)
    {
        // 境界と同じxの点はこの時点ではまだ加算しない。
        while (next < ordered.size() && ordered[next].x < x)
        {
            const auto &[px, rank, weight] = ordered[next++];
            for (auto p = static_cast<std::size_t>(rank); p < tree.size(); p |= p + 1)
            {
                tree[p] += static_cast<std::uint64_t>(weight);
            }
        }
        const auto sum = prefix(end) - prefix(begin);
        auto &value = answer[id_and_side / 2];
        const auto unsigned_value = std::bit_cast<std::uint64_t>(value);
        value = std::bit_cast<std::int64_t>(id_and_side % 2 == 0 ? unsigned_value - sum : unsigned_value + sum);
    }
    return answer;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
