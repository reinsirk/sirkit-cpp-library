// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Static_Cuboid_Sum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/static_cuboid_sum.md
//
// API一覧
// StaticCuboidSumPoint{x,y,z,weight}: 3次元の重み付き点。座標・重みは符号付き64bit。同じ座標の点も加算する。
// StaticCuboidSumQuery{lower,upper}: 配列{x,y,z}で指定した直方体。各軸でlowerを含みupperを含まない。
// static_cuboid_sum(points,queries): 各直方体内の重みの総和を照会順に返す。空直方体は0。
// 重みの総和は2^64を法とし、符号付き64bitの値として返す。検査有効時は逆転範囲でinvalid_argument。
// 点N個・照会Q個について O((N+Q)log²(N+Q+2))時間、O(N+Q)追加領域。
//
// [同梱の点加算・長方形和]
// PointAdd{x,y,weight}: 重み付き点を追加する。同じ位置に追加した重みも含まれる。
// RectangleSumRequest{left,bottom,right,top}: 半開長方形内を集計する。空長方形の答えは0。
// PointAddRectangleSumOperation: 上記2種類の操作を表すstd::variant。
// offline_point_add_rectangle_sum(initial,operations): 初期点と各照会までの追加点を集計し、照会順に返す。
// 座標・重みは符号付き64ビット。合計は2^64を法とし、符号付き64ビット値へ対応させる。
// 初期点n・操作qについて決定的O((n+q)log²(n+q+2))時間、O(n+q)領域。


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
#include <array>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_OFFLINE_POINT_ADD_RECTANGLE_SUM_DEFINED
#define SIRKIT_OFFLINE_POINT_ADD_RECTANGLE_SUM_DEFINED

struct PointAdd
{
    std::int64_t x, y, weight;
};

struct RectangleSumRequest
{
    std::int64_t left, bottom, right, top;
};

using PointAddRectangleSumOperation = std::variant<PointAdd, RectangleSumRequest>;

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t CheckedPointAddRectangleSumEventCount(std::size_t initial, std::size_t updates,
                                                         std::size_t queries, std::size_t limit)
{
    if (queries > limit / 2)
    {
        throw std::length_error("offline_point_add_rectangle_sum: too many queries");
    }
    const auto room = limit - 2 * queries;
    if (initial > room || updates > room - initial)
    {
        throw std::length_error("offline_point_add_rectangle_sum: too many points");
    }
    return initial + updates + 2 * queries;
}
}

inline std::vector<std::int64_t> offline_point_add_rectangle_sum(
    std::span<const PointAdd> initial, std::span<const PointAddRectangleSumOperation> operations)
{
    using Index = std::uint32_t;
    constexpr auto kPoint = std::numeric_limits<Index>::max();
    struct Event
    {
        std::int64_t x;
        std::uint64_t weight_or_query;
        Index first, last;
    };

    std::size_t updates = 0;
    for (const auto &operation : operations)
    {
        if (std::holds_alternative<PointAdd>(operation))
        {
            ++updates;
        }
    }
    const auto queries = operations.size() - updates;
    if (queries == 0)
    {
        return {};
    }

    std::vector<Event> events;
    // lastの最大値を点追加の印として予約する。
    const auto limit = std::min<std::size_t>(kPoint - 1, events.max_size());
    const auto capacity = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedPointAddRectangleSumEventCount(
        initial.size(), updates, queries, limit);

    std::vector<std::int64_t> ys;
    ys.reserve(initial.size() + updates);
    for (const auto &point : initial)
    {
        ys.push_back(point.y);
    }
    for (const auto &operation : operations)
    {
        if (const auto *point = std::get_if<PointAdd>(&operation))
        {
            ys.push_back(point->y);
        }
    }
    std::vector<std::int64_t> result(queries);
    if (ys.empty())
    {
        return result;
    }
    std::sort(ys.begin(), ys.end());
    ys.erase(std::unique(ys.begin(), ys.end()), ys.end());

    const auto rank = [&](std::int64_t y)
    {
        return static_cast<Index>(std::lower_bound(ys.begin(), ys.end(), y) - ys.begin());
    };
    events.reserve(capacity);
    const auto append = [&](const PointAdd &point)
    {
        if (point.weight != 0)
        {
            events.push_back({point.x, static_cast<std::uint64_t>(point.weight), rank(point.y), kPoint});
        }
    };
    for (const auto &point : initial)
    {
        append(point);
    }
    const auto first_dynamic = events.size();
    Index query_id = 0;
    std::size_t query_events = 0;
    for (const auto &operation : operations)
    {
        if (const auto *point = std::get_if<PointAdd>(&operation))
        {
            append(*point);
            continue;
        }
        const auto id = query_id++;
        const auto &rectangle = std::get<RectangleSumRequest>(operation);
        const auto first = rank(rectangle.bottom);
        const auto last = rank(rectangle.top);
        if (rectangle.left == rectangle.right || first == last)
        {
            continue;
        }
        const auto code = std::uint64_t{id} * 2;
        events.push_back({rectangle.left, code + 1, first, last});
        events.push_back({rectangle.right, code, first, last});
        query_events += 2;
    }
    if (query_events == 0 || events.size() == query_events)
    {
        return result;
    }

    std::vector<std::uint64_t> fenwick(ys.size());
    const auto update = [&](std::size_t rank_y, std::uint64_t delta)
    {
        for (; rank_y < fenwick.size(); rank_y |= rank_y + 1)
        {
            fenwick[rank_y] += delta;
        }
    };
    const auto prefix = [&](std::size_t count)
    {
        std::uint64_t value = 0;
        for (; count != 0; count &= count - 1)
        {
            value += fenwick[count - 1];
        }
        return value;
    };
    const auto sweep = [&](std::size_t left, std::size_t middle, std::size_t right)
    {
        auto cursor = left;
        for (auto i = middle; i < right; ++i)
        {
            const auto &query = events[i];
            if (query.last == kPoint)
            {
                continue;
            }
            while (cursor < middle && events[cursor].x < query.x)
            {
                const auto &point = events[cursor++];
                if (point.last == kPoint)
                {
                    update(point.first, point.weight_or_query);
                }
            }
            const auto sum = prefix(query.last) - prefix(query.first);
            const auto code = query.weight_or_query;
            auto &answer = result[code / 2];
            const auto delta = (code & 1) ? std::uint64_t{0} - sum : sum;
            answer = std::bit_cast<std::int64_t>(std::bit_cast<std::uint64_t>(answer) + delta);
        }
        for (auto i = left; i < cursor; ++i)
        {
            const auto &point = events[i];
            if (point.last == kPoint)
            {
                update(point.first, std::uint64_t{0} - point.weight_or_query);
            }
        }
    };

    const auto by_x = [](const Event &a, const Event &b)
    {
        return a.x < b.x;
    };
    if (events.size() - first_dynamic == query_events)
    {
        std::sort(events.begin() + first_dynamic, events.end(), by_x);
    }
    else
    {
        std::vector<Event> scratch(events.size() - first_dynamic);
        const auto divide = [&](auto &&self, std::size_t left, std::size_t right) -> unsigned
        {
            constexpr unsigned kHasPoint = 1, kHasQuery = 2;
            if (right - left == 1)
            {
                return events[left].last == kPoint ? kHasPoint : kHasQuery;
            }
            const auto middle = left + (right - left) / 2;
            const auto before = self(self, left, middle);
            const auto after = self(self, middle, right);
            if ((before & kHasPoint) && (after & kHasQuery))
            {
                sweep(left, middle, right);
            }
            if (events[middle - 1].x > events[middle].x)
            {
                const auto first = events.begin() + left;
                const auto split = events.begin() + middle;
                const auto end = events.begin() + right;
                const auto produced = std::merge(first, split, split, end, scratch.begin(), by_x);
                std::copy(scratch.begin(), produced, first);
            }
            return before | after;
        };
        divide(divide, first_dynamic, events.size());
    }
    if (first_dynamic != 0)
    {
        std::sort(events.begin(), events.begin() + first_dynamic, by_x);
        sweep(0, first_dynamic, events.size());
    }
    return result;
}

#endif

#ifndef SIRKIT_STATIC_CUBOID_SUM_DEFINED
#define SIRKIT_STATIC_CUBOID_SUM_DEFINED

struct StaticCuboidSumPoint
{
    std::int64_t x, y, z, weight;
};

struct StaticCuboidSumQuery
{
    std::array<std::int64_t, 3> lower;
    std::array<std::int64_t, 3> upper;
};

inline std::vector<std::int64_t> static_cuboid_sum(std::span<const StaticCuboidSumPoint> points,
                                                   std::span<const StaticCuboidSumQuery> queries)
{
    struct Boundary
    {
        std::int64_t x;
        std::size_t query;
        bool subtract;
    };

    std::vector<std::int64_t> answer(queries.size());
    std::vector<Boundary> boundaries;
    boundaries.reserve(detail::CheckedPointAddRectangleSumEventCount(0, 0, queries.size(), boundaries.max_size()));
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        const auto &[lower, upper] = queries[i];
        if (lower[0] < upper[0] && lower[1] < upper[1] && lower[2] < upper[2])
        {
            boundaries.push_back({lower[0], i, true});
            boundaries.push_back({upper[0], i, false});
        }
    }
    if (boundaries.empty() || points.empty())
    {
        return answer;
    }

    auto ordered = std::vector(points.begin(), points.end());
    std::ranges::sort(ordered, {}, &StaticCuboidSumPoint::x);
    std::ranges::sort(boundaries, {}, &Boundary::x);
    std::vector<PointAddRectangleSumOperation> operations;
    operations.reserve(detail::CheckedPointAddRectangleSumEventCount(
        0, ordered.size(), boundaries.size() / 2, operations.max_size()));
    auto next_point = ordered.begin();
    for (const auto &boundary : boundaries)
    {
        // xが等しい点はprefixに加えない。半開区間ではその点は右側に含まれる。
        while (next_point != ordered.end() && next_point->x < boundary.x)
        {
            operations.emplace_back(PointAdd{next_point->y, next_point->z, next_point->weight});
            ++next_point;
        }
        const auto &[lower, upper] = queries[boundary.query];
        operations.emplace_back(RectangleSumRequest{lower[1], lower[2], upper[1], upper[2]});
    }
    const auto sums = offline_point_add_rectangle_sum({}, operations);
    for (std::size_t i = 0; i < boundaries.size(); ++i)
    {
        const auto &boundary = boundaries[i];
        auto weight = static_cast<std::uint64_t>(sums[i]);
        if (boundary.subtract)
        {
            weight = std::uint64_t{0} - weight;
        }
        auto &total = answer[boundary.query];
        total = static_cast<std::int64_t>(static_cast<std::uint64_t>(total) + weight);
    }
    return answer;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
