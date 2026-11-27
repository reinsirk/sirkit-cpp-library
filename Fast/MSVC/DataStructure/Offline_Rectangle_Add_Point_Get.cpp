// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Offline_Rectangle_Add_Point_Get.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/offline_rectangle_add_point_get.md
//
// API一覧
// RectangleAdd{left,bottom,right,top,weight}: 半開長方形[left,right)×[bottom,top)へ重みを加える。
// RectanglePointGet{x,y}: 点を含む長方形の重みの和を問い合わせる。
// RectangleAddPointGetOperation: 上の2操作を保持するstd::variant。
// offline_rectangle_add_point_get(initial,operations): 初期長方形を置いて操作を順に適用し、照会の答えを照会順に返す。
// 座標と重みは符号付き64ビット。幅または高さ0の長方形は無視する。同じ長方形を重ねて追加できる。
// 累積値は2^64を法とし、符号付き64ビットへ対応付けて返す。検査有効時、逆順の境界はinvalid_argument。
// 初期長方形数n・操作数qに対しO((n+q)log²(n+q+2))時間・O(n+q)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <variant>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_OFFLINE_RECTANGLE_ADD_POINT_GET_DEFINED
#define SIRKIT_OFFLINE_RECTANGLE_ADD_POINT_GET_DEFINED

struct RectangleAdd
{
    std::int64_t left, bottom, right, top, weight;
};

struct RectanglePointGet
{
    std::int64_t x, y;
};

using RectangleAddPointGetOperation = std::variant<RectangleAdd, RectanglePointGet>;

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t CheckedRectangleAddPointGetEventCount(std::size_t initial, std::size_t updates,
                                                         std::size_t queries, std::size_t limit)
{
    if (queries > limit)
    {
        throw std::length_error("offline_rectangle_add_point_get: too many queries");
    }
    const auto rectangle_limit = (limit - queries) / 2;
    if (initial > rectangle_limit || updates > rectangle_limit - initial)
    {
        throw std::length_error("offline_rectangle_add_point_get: too many rectangles");
    }
    return 2 * (initial + updates) + queries;
}
} // namespace detail

inline std::vector<std::int64_t> offline_rectangle_add_point_get(
    std::span<const RectangleAdd> initial, std::span<const RectangleAddPointGetOperation> operations)
{
    using Index = std::uint32_t;
    struct Event
    {
        std::int64_t x;
        std::uint64_t weight;
        Index begin, end_or_answer;
    };


    std::vector<std::int64_t> ys;
    for (const auto &operation : operations)
    {
        if (const auto *point = std::get_if<RectanglePointGet>(&operation))
        {
            ys.push_back(point->y);
        }
    }
    const auto queries = ys.size();
    if (queries == 0)
    {
        return {};
    }

    std::vector<Event> events;
    const auto capacity = std::min<std::size_t>(std::numeric_limits<Index>::max(), events.max_size());
    events.reserve(detail::CheckedRectangleAddPointGetEventCount(
        initial.size(), operations.size() - queries, queries, capacity));
    std::sort(ys.begin(), ys.end());
    ys.erase(std::unique(ys.begin(), ys.end()), ys.end());

    const auto rank = [&](std::int64_t ordinate)
    {
        return static_cast<Index>(std::lower_bound(ys.begin(), ys.end(), ordinate) - ys.begin());
    };
    const auto append = [&](const RectangleAdd &rectangle)
    {
        const auto [left, bottom, right, top, weight] = rectangle;
        if (weight == 0 || left == right)
        {
            return;
        }
        const auto begin = rank(bottom);
        const auto end = rank(top);
        if (begin == end)
        {
            return;
        }
        const auto delta = static_cast<std::uint64_t>(weight);
        events.push_back({left, delta, begin, end});
        events.push_back({right, std::uint64_t{0} - delta, begin, end});
    };
    for (const auto &rectangle : initial)
    {
        append(rectangle);
    }
    const auto initial_events = events.size();
    Index query_id = 0;
    for (const auto &operation : operations)
    {
        if (const auto *point = std::get_if<RectanglePointGet>(&operation))
        {
            // 重み0の追加は登録しないので、weight=0を照会の印にできる。
            events.push_back({point->x, 0, rank(point->y), query_id++});
        }
        else
        {
            append(std::get<RectangleAdd>(operation));
        }
    }

    std::vector<std::int64_t> result(queries);
    if (events.size() == queries)
    {
        return result;
    }

    std::vector<std::uint64_t> fenwick(ys.size());
    const auto update = [&](std::size_t index, std::uint64_t delta)
    {
        for (; index < fenwick.size(); index |= index + 1)
        {
            fenwick[index] += delta;
        }
    };
    const auto add = [&](const Event &event, std::uint64_t delta)
    {
        update(event.begin, delta);
        update(event.end_or_answer, std::uint64_t{0} - delta);
    };
    const auto get = [&](std::size_t index)
    {
        std::uint64_t value = 0;
        for (auto end = index + 1; end != 0; end &= end - 1)
        {
            value += fenwick[end - 1];
        }
        return value;
    };

    // 各追加と、時間順で後ろにある照会の組を分割の境界で1回だけ処理する。
    const auto sweep = [&](std::size_t left, std::size_t middle, std::size_t right)
    {
        auto cursor = left;
        for (auto i = middle; i < right; ++i)
        {
            const auto &query = events[i];
            if (query.weight != 0)
            {
                continue;
            }
            while (cursor < middle && events[cursor].x <= query.x)
            {
                const auto &event = events[cursor++];
                if (event.weight != 0)
                {
                    add(event, event.weight);
                }
            }
            auto &answer = result[query.end_or_answer];
            answer = std::bit_cast<std::int64_t>(std::bit_cast<std::uint64_t>(answer) + get(query.begin));
        }
        for (auto i = left; i < cursor; ++i)
        {
            const auto &event = events[i];
            if (event.weight != 0)
            {
                add(event, std::uint64_t{0} - event.weight);
            }
        }
    };
    const auto by_x = [](const Event &a, const Event &b)
    {
        return a.x < b.x;
    };
    std::vector<Event> scratch(events.size() - initial_events);
    const auto divide = [&](auto &&self, std::size_t left, std::size_t right) -> void
    {
        if (right - left <= 1)
        {
            return;
        }
        const auto middle = left + (right - left) / 2;
        self(self, left, middle);
        self(self, middle, right);
        sweep(left, middle, right);

        if (events[middle - 1].x > events[middle].x)
        {
            const auto first = events.begin() + left;
            const auto split = events.begin() + middle;
            const auto last = events.begin() + right;
            const auto end = std::merge(first, split, split, last, scratch.begin(), by_x);
            std::copy(scratch.begin(), end, first);
        }
    };

    divide(divide, initial_events, events.size());
    if (initial_events != 0)
    {
        std::sort(events.begin(), events.begin() + initial_events, by_x);
        sweep(0, initial_events, events.size());
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
