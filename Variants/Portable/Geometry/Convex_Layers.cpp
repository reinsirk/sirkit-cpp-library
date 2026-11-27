// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Geometry/Convex_Layers.cpp
// 参考: M. H. Overmars, J. van Leeuwen, Maintenance of configurations in the plane
// (https://doi.org/10.1016/0022-0000(81)90012-X)。接線探索を含むコードは独自実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/convex_layers.md
//
// API一覧
// convex_layers(points): 凸包の境界上にあるすべての点を取り除く操作を繰り返し、各点の削除回を返す。
// 戻り値は入力順、最初の層は1。同じ座標の点は同じ層。辺上の点や一直線上の残点も同時に取り除く。
// x、yそれぞれの最大値と最小値の差は10^6以下。超える場合はoverflow_error。
// O(N log²(N+1))時間・O(N)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CONVEX_LAYERS_DEFINED
#define SIRKIT_CONVEX_LAYERS_DEFINED

namespace detail
{
using ConvexLayerPoint = std::pair<std::int64_t, std::int64_t>;

class ConvexLayerHull
{
    using Index = std::size_t;

    struct Bridge
    {
        Index branch = 0;
        Index first = 0;
        Index last = 0;
    };

public:
    ConvexLayerHull(std::span<const ConvexLayerPoint> points, std::int64_t direction)
        : points_(points), direction_(direction), leaves_(std::bit_ceil(points.size())), tree_(2 * leaves_)
    {
        for (Index i = 0; i < points_.size(); ++i)
        {
            tree_[leaves_ + i] = {leaves_ + i, i, i};
        }
        for (auto node = leaves_; node > 1;)
        {
            --node;
            Repair(node);
        }
    }

    bool Empty() const
    {
        return tree_[1].branch == 0;
    }

    void Boundary(std::vector<Index> &boundary) const
    {
        Collect(tree_[1], 0, points_.size() - 1, boundary);
    }

    void Erase(Index point)
    {
        auto node = leaves_ + point;
        tree_[node] = {};
        while (node > 1)
        {
            node /= 2;
            const auto bridge = tree_[node];
            if (bridge.branch != node || point == bridge.first || point == bridge.last)
            {
                Repair(node);
            }
        }
    }

private:
    std::span<const ConvexLayerPoint> points_;
    std::int64_t direction_;
    Index leaves_;
    std::vector<Bridge> tree_;

    Bridge Child(Bridge hull, bool right) const
    {
        return tree_[2 * hull.branch + right];
    }

    static std::int64_t Cross(const ConvexLayerPoint &a, const ConvexLayerPoint &b, const ConvexLayerPoint &c)
    {
        return (b.first - a.first) * (c.second - a.second) - (b.second - a.second) * (c.first - a.first);
    }

    void Collect(Bridge bridge, Index first, Index last, std::vector<Index> &boundary) const
    {
        // 範囲はこの凸包上の点を少なくとも1つ含む。葉まで来たらその点を追加する。
        if (bridge.branch >= leaves_)
        {
            boundary.push_back(bridge.first);
            return;
        }
        if (first <= bridge.first)
        {
            Collect(Child(bridge, false), first, std::min(last, bridge.first), boundary);
        }
        if (bridge.last <= last)
        {
            Collect(Child(bridge, true), std::max(first, bridge.last), last, boundary);
        }
    }

    std::int64_t CompareHeight(const ConvexLayerPoint &a, const ConvexLayerPoint &b,
                               const ConvexLayerPoint &c, const ConvexLayerPoint &d,
                               const ConvexLayerPoint &midpoint_twice) const
    {
        const auto height_numerator = [&](const ConvexLayerPoint &from, const ConvexLayerPoint &to)
        {
            return (2 * from.second - midpoint_twice.second) * (to.first - from.first) +
                   (midpoint_twice.first - 2 * from.first) * (to.second - from.second);
        };
        const auto ab = height_numerator(a, b);
        const auto cd = height_numerator(c, d);
        // x+εyで厳密に左右分離する。分母を払った多項式の定数項、εの係数の順に調べる。
        const auto ordinary = ab * (d.first - c.first) - cd * (b.first - a.first);
        const auto comparison = ordinary != 0 ? ordinary : ab * (d.second - c.second) - cd * (b.second - a.second);
        return ((comparison > 0) - (comparison < 0)) * direction_;
    }

    Bridge FindBridge(Index node, Bridge left, Bridge right) const
    {
        const auto child_width = leaves_ >> std::bit_width(node);
        const auto split = (2 * node + 1) * child_width - leaves_;
        const ConvexLayerPoint midpoint_twice{points_[split - 1].first + points_[split].first,
                                              points_[split - 1].second + points_[split].second};

        // 分割位置での支持直線の高さは、傾きを増すと左の凸包で増加し右の凸包で減少する。
        // 2つの辺の傾きと高さを比較し、共通接線の接点を含む子へ下りる。
        while (left.first != left.last || right.first != right.last)
        {
            const auto &a = points_[left.first], &b = points_[left.last];
            const auto &c = points_[right.first], &d = points_[right.last];
            if (left.first == left.last)
            {
                right = Child(right, direction_ * Cross(c, d, a) < 0);
                continue;
            }
            if (right.first == right.last)
            {
                left = Child(left, direction_ * Cross(a, b, c) >= 0);
                continue;
            }

            const auto slope_order = direction_ *
                                     ((b.second - a.second) * (d.first - c.first) -
                                      (d.second - c.second) * (b.first - a.first));
            const auto height_order = CompareHeight(a, b, c, d, midpoint_twice);
            if (slope_order == 0 && height_order == 0)
            {
                // 同一直線では、境界上の点を隠さない最も短い橋を選ぶ。
                left = Child(left, true);
                right = Child(right, false);
            }
            else if (slope_order <= 0)
            {
                if (height_order <= 0)
                {
                    left = Child(left, true);
                }
                else
                {
                    right = Child(right, false);
                }
            }
            else if (height_order <= 0)
            {
                right = Child(right, true);
            }
            else
            {
                left = Child(left, false);
            }
        }
        return {node, left.first, right.first};
    }

    void Repair(Index node)
    {
        const auto left = tree_[2 * node];
        const auto right = tree_[2 * node + 1];
        if (left.branch == 0)
        {
            tree_[node] = right;
        }
        else if (right.branch == 0)
        {
            tree_[node] = left;
        }
        else
        {
            tree_[node] = FindBridge(node, left, right);
        }
    }
};
} // namespace detail

inline std::vector<std::size_t> convex_layers(std::span<const std::pair<std::int64_t, std::int64_t>> points)
{
    if (points.empty())
    {
        return {};
    }

    const auto min_x = std::ranges::min_element(points, {}, &detail::ConvexLayerPoint::first)->first;
    const auto min_y = std::ranges::min_element(points, {}, &detail::ConvexLayerPoint::second)->second;
    constexpr std::uint64_t kMaxSpan = 1'000'000;
    const auto translate = [&](std::int64_t value, std::int64_t minimum)
    {
        const auto difference = static_cast<std::uint64_t>(value) - static_cast<std::uint64_t>(minimum);
        if (difference > kMaxSpan)
        {
            throw std::overflow_error("convex_layers: coordinate range is too wide");
        }
        return static_cast<std::int64_t>(difference);
    };
    std::vector<std::size_t> order(points.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    std::ranges::sort(order, {}, [&](std::size_t i)
                      {
                          return points[i];
                      });
    std::vector<detail::ConvexLayerPoint> unique;
    unique.reserve(points.size());
    std::vector<std::size_t> rank(points.size());
    for (const auto i : order)
    {
        const auto &[x, y] = points[i];
        const detail::ConvexLayerPoint point{translate(x, min_x), translate(y, min_y)};
        if (unique.empty() || unique.back() != point)
        {
            unique.push_back(point);
        }
        rank[i] = unique.size() - 1;
    }

    detail::ConvexLayerHull lower(unique, 1), upper(unique, -1);
    std::vector<std::size_t> layers(unique.size()), boundary;
    for (std::size_t layer = 1; !lower.Empty(); ++layer)
    {
        boundary.clear();
        lower.Boundary(boundary);
        upper.Boundary(boundary);
        for (const auto point : boundary)
        {
            if (layers[point] == 0)
            {
                layers[point] = layer;
                lower.Erase(point);
                upper.Erase(point);
            }
        }
    }
    for (auto &point : rank)
    {
        point = layers[point];
    }
    return rank;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
