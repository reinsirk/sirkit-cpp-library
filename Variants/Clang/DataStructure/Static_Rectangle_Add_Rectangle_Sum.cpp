// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Static_Rectangle_Add_Rectangle_Sum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/static_rectangle_add_rectangle_sum.md
//
// API一覧
// StaticRectangleAdd{left,bottom,right,top,weight}: 半開長方形内の各格子点へ重みを加える。
// StaticRectangleSumQuery{left,bottom,right,top}: 半開長方形内の格子点の総和を照会する。
// static_rectangle_add_rectangle_sum<Mod>(additions,queries): 全追加後の和を照会順に返す。
// 座標と重みは符号付き64ビット。幅・高さ0の追加は無視し、幅・高さ0の照会は0。
// 合計はModでの非負剰余。Mod既定値は998244353。素数でなくてよい。
// 追加n件・照会q件についてO((n+q)log(n+q+1))時間、O(n+q)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STATIC_RECTANGLE_ADD_RECTANGLE_SUM_DEFINED
#define SIRKIT_STATIC_RECTANGLE_ADD_RECTANGLE_SUM_DEFINED

struct StaticRectangleAdd
{
    std::int64_t left, bottom, right, top, weight;
};

struct StaticRectangleSumQuery
{
    std::int64_t left, bottom, right, top;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t CheckedStaticRectangleAreaDoubleCount(std::size_t count, std::size_t limit)
{
    if (count > limit / 2)
    {
        throw std::length_error("static_rectangle_add_rectangle_sum: too many events");
    }
    return 2 * count;
}

template <std::uint32_t Mod>
struct StaticRectangleAreaArithmetic
{
    static_assert(Mod > 0);

    static std::uint32_t Normalize(std::int64_t value)
    {
        const auto residue = value % static_cast<std::int64_t>(Mod);
        return static_cast<std::uint32_t>(residue < 0 ? residue + Mod : residue);
    }

    static std::uint32_t Add(std::uint32_t a, std::uint32_t b)
    {
        const auto sum = static_cast<std::uint64_t>(a) + b;
        return static_cast<std::uint32_t>(sum >= Mod ? sum - Mod : sum);
    }

    static std::uint32_t Subtract(std::uint32_t a, std::uint32_t b)
    {
        return a >= b ? a - b : a + (Mod - b);
    }

    static std::uint32_t Multiply(std::uint32_t a, std::uint32_t b)
    {
        return static_cast<std::uint32_t>((static_cast<std::uint64_t>(a) * b) % Mod);
    }
};
} // namespace reinsirk::detail

template <std::uint32_t Mod = 998244353>
std::vector<std::uint32_t> static_rectangle_add_rectangle_sum(
    std::span<const StaticRectangleAdd> additions, std::span<const StaticRectangleSumQuery> queries)
{
    using Arithmetic = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::StaticRectangleAreaArithmetic<Mod>;
    using Index = std::uint32_t;
    using Moments = std::array<std::uint32_t, 4>;

    struct AddEvent
    {
        std::int64_t x;
        Index bottom, top;
        std::uint32_t weight;
    };

    struct QueryEvent
    {
        std::int64_t x;
        std::size_t encoded_query;
    };

    struct QueryY
    {
        Index bottom, top;
        std::uint32_t bottom_mod, top_mod;
    };

#ifndef SKIP_RUNTIME_CHECKS
    const auto check = [](const auto &rectangle)
    {
        if (rectangle.left > rectangle.right || rectangle.bottom > rectangle.top)
        {
            throw std::invalid_argument("static_rectangle_add_rectangle_sum: reversed rectangle");
        }
    };
#endif

    std::vector<std::int64_t> ys;
    ys.reserve(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedStaticRectangleAreaDoubleCount(
        additions.size(), std::min<std::size_t>(std::numeric_limits<Index>::max(), ys.max_size())));
    for (const auto &rectangle : additions)
    {
#ifndef SKIP_RUNTIME_CHECKS
        check(rectangle);
#endif
        if (rectangle.left != rectangle.right && rectangle.bottom != rectangle.top &&
            Arithmetic::Normalize(rectangle.weight) != 0)
        {
            ys.push_back(rectangle.bottom);
            ys.push_back(rectangle.top);
        }
    }
    std::sort(ys.begin(), ys.end());
    ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
    const auto rank = [&](std::int64_t y)
    {
        return static_cast<Index>(std::lower_bound(ys.begin(), ys.end(), y) - ys.begin());
    };

    std::vector<QueryEvent> query_events;
    query_events.reserve(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedStaticRectangleAreaDoubleCount(queries.size(), query_events.max_size()));
    std::vector<QueryY> query_ys(queries.size());
    for (std::size_t id = 0; id < queries.size(); ++id)
    {
        const auto &rectangle = queries[id];
#ifndef SKIP_RUNTIME_CHECKS
        check(rectangle);
#endif
        if (rectangle.left == rectangle.right || rectangle.bottom == rectangle.top || ys.empty())
        {
            continue;
        }
        query_ys[id] = {rank(rectangle.bottom), rank(rectangle.top),
                        Arithmetic::Normalize(rectangle.bottom), Arithmetic::Normalize(rectangle.top)};
        // 偶数は左辺で減算、奇数は右辺で加算する。
        query_events.push_back({rectangle.left, 2 * id});
        query_events.push_back({rectangle.right, 2 * id + 1});
    }

    std::vector<std::uint32_t> result(queries.size());
    if (query_events.empty())
    {
        return result;
    }

    std::vector<AddEvent> add_events;
    add_events.reserve(/*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedStaticRectangleAreaDoubleCount(additions.size(), add_events.max_size()));
    for (const auto &rectangle : additions)
    {
        const auto weight = Arithmetic::Normalize(rectangle.weight);
        if (rectangle.left == rectangle.right || rectangle.bottom == rectangle.top || weight == 0)
        {
            continue;
        }
        const auto bottom = rank(rectangle.bottom);
        const auto top = rank(rectangle.top);
        add_events.push_back({rectangle.left, bottom, top, weight});
        add_events.push_back({rectangle.right, bottom, top, Arithmetic::Subtract(0, weight)});
    }
    std::ranges::sort(add_events, {}, &AddEvent::x);
    std::ranges::sort(query_events, {}, &QueryEvent::x);

    std::vector<std::uint32_t> ys_mod(ys.size());
    std::transform(ys.begin(), ys.end(), ys_mod.begin(), Arithmetic::Normalize);
    std::vector<Moments> fenwick(ys.size());
    const auto update = [&](std::size_t index, std::uint32_t weight, std::uint32_t weight_x)
    {
        const Moments delta{weight, weight_x, Arithmetic::Multiply(weight, ys_mod[index]),
                            Arithmetic::Multiply(weight_x, ys_mod[index])};
        for (; index < fenwick.size(); index |= index + 1)
        {
            auto &node = fenwick[index];
            for (std::size_t i = 0; i < node.size(); ++i)
            {
                node[i] = Arithmetic::Add(node[i], delta[i]);
            }
        }
    };
    const auto prefix_area = [&](std::size_t count, std::uint32_t x, std::uint32_t y)
    {
        Moments sum{};
        while (count != 0)
        {
            const auto &node = fenwick[count - 1];
            for (std::size_t i = 0; i < node.size(); ++i)
            {
                sum[i] = Arithmetic::Add(sum[i], node[i]);
            }
            count &= count - 1;
        }
        // 左下の角(x0,y0)からの寄与w(x-x0)(y-y0)を4項に展開する。
        const auto first = Arithmetic::Subtract(Arithmetic::Multiply(sum[0], x), sum[1]);
        const auto second = Arithmetic::Subtract(Arithmetic::Multiply(sum[2], x), sum[3]);
        return Arithmetic::Subtract(Arithmetic::Multiply(first, y), second);
    };

    std::size_t next = 0;
    for (const auto &event : query_events)
    {
        while (next < add_events.size() && add_events[next].x < event.x)
        {
            const auto &addition = add_events[next++];
            const auto weight_x = Arithmetic::Multiply(addition.weight, Arithmetic::Normalize(addition.x));
            update(addition.bottom, addition.weight, weight_x);
            update(addition.top, Arithmetic::Subtract(0, addition.weight), Arithmetic::Subtract(0, weight_x));
        }

        const auto id = event.encoded_query / 2;
        const auto &y = query_ys[id];
        const auto x = Arithmetic::Normalize(event.x);
        const auto slice = Arithmetic::Subtract(prefix_area(y.top, x, y.top_mod),
                                                prefix_area(y.bottom, x, y.bottom_mod));
        if (event.encoded_query % 2 == 0)
        {
            result[id] = Arithmetic::Subtract(result[id], slice);
        }
        else
        {
            result[id] = Arithmetic::Add(result[id], slice);
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
