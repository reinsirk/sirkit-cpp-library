// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Euclidean_MST.cpp
// 参考: L. Guibas, J. Stolfi, Primitives for the Manipulation of General Subdivisions and the
// Computation of Voronoi Diagrams (https://doi.org/10.1145/282918.282923)。コードは独自実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/euclidean_mst.md
//
// API一覧
// euclidean_mst(points): 整数座標の完全グラフで、辺の重みをユークリッド距離とした最小全域木を返す。
// EuclideanMstEdge{from, to, distance_squared}: 元の点の0始まり添字と距離の二乗。
// 同じ点の間の距離は0。同距離の辺が複数あれば任意の最小全域木を返す。0点・1点では空。
// 各軸の座標幅は25,000以下。超える場合はoverflow_error。O(N log(N+1))時間・O(N)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EUCLIDEAN_MST_DEFINED
#define SIRKIT_EUCLIDEAN_MST_DEFINED

struct EuclideanMstEdge
{
    std::size_t from;
    std::size_t to;
    std::uint64_t distance_squared;
};

namespace detail
{
using EuclideanMstPoint = std::pair<std::int64_t, std::int64_t>;

inline EuclideanMstPoint EuclideanMstDifference(const EuclideanMstPoint &a, const EuclideanMstPoint &b)
{
    return {a.first - b.first, a.second - b.second};
}

inline std::int64_t EuclideanMstCross(const EuclideanMstPoint &a, const EuclideanMstPoint &b)
{
    return a.first * b.second - a.second * b.first;
}

inline std::int64_t EuclideanMstNormSquared(const EuclideanMstPoint &p)
{
    return p.first * p.first + p.second * p.second;
}

class EuclideanMstTriangulation
{
    using Index = std::size_t;
    static constexpr Index kNone = std::numeric_limits<Index>::max();

    struct HalfEdge
    {
        Index next;
        Index origin;
    };

public:
    explicit EuclideanMstTriangulation(std::span<const EuclideanMstPoint> points) : points_(points)
    {
        edges_.reserve(12 * points.size());
        Build(0, points.size());
    }

    std::vector<EuclideanMstEdge> WeightedEdges() const
    {
        std::vector<EuclideanMstEdge> result;
        result.reserve(edges_.size() / 4);
        // 構築途中も辺は交差しない。完成した三角形分割が最大辺数に達するため全ブロックが使用中。
        for (Index i = 0; i < edges_.size(); i += 4)
        {
            const auto from = Origin(i), to = Destination(i);
            const auto delta = EuclideanMstDifference(points_[from], points_[to]);
            result.push_back({from, to, static_cast<std::uint64_t>(EuclideanMstNormSquared(delta))});
        }
        return result;
    }

private:
    std::span<const EuclideanMstPoint> points_;
    std::vector<HalfEdge> edges_;
    Index free_ = kNone;

    static Index Reverse(Index edge)
    {
        return edge ^ Index{2};
    }

    static Index Rotate(Index edge)
    {
        return (edge & ~Index{3}) | ((edge + 1) & Index{3});
    }

    static Index RotateBack(Index edge)
    {
        return Reverse(Rotate(edge));
    }

    Index Origin(Index edge) const
    {
        return edges_[edge].origin;
    }

    Index Destination(Index edge) const
    {
        return Origin(Reverse(edge));
    }

    Index AroundOrigin(Index edge) const
    {
        return edges_[edge].next;
    }

    Index AroundOriginBack(Index edge) const
    {
        return Rotate(AroundOrigin(Rotate(edge)));
    }

    Index AroundLeftFace(Index edge) const
    {
        return Rotate(AroundOrigin(RotateBack(edge)));
    }

    Index Add(Index from, Index to)
    {
        const auto base = free_ == kNone ? edges_.size() : free_;
        const std::array<HalfEdge, 4> block{{{base, from}, {base + 3, kNone}, {base + 2, to}, {base + 1, kNone}}};
        if (free_ == kNone)
        {
            edges_.insert(edges_.end(), block.begin(), block.end());
        }
        else
        {
            free_ = edges_[base].next;
            std::ranges::copy(block, edges_.begin() + base);
        }
        return base;
    }

    void Splice(Index a, Index b)
    {
        const auto dual_a = Rotate(AroundOrigin(a));
        const auto dual_b = Rotate(AroundOrigin(b));
        std::swap(edges_[a].next, edges_[b].next);
        std::swap(edges_[dual_a].next, edges_[dual_b].next);
    }

    Index Connect(Index a, Index b)
    {
        const auto edge = Add(Destination(a), Origin(b));
        Splice(edge, AroundLeftFace(a));
        Splice(Reverse(edge), b);
        return edge;
    }

    void Erase(Index edge)
    {
        Splice(edge, AroundOriginBack(edge));
        const auto opposite = Reverse(edge);
        Splice(opposite, AroundOriginBack(opposite));
        const auto base = edge & ~Index{3};
        edges_[base] = {free_, kNone};
        free_ = base;
    }

    std::int64_t Side(Index edge, Index point) const
    {
        const auto &from = points_[Origin(edge)];
        const auto to = EuclideanMstDifference(points_[Destination(edge)], from);
        return EuclideanMstCross(to, EuclideanMstDifference(points_[point], from));
    }

    bool InCircle(Index a, Index b, Index c, Index test) const
    {
        const auto x = EuclideanMstDifference(points_[a], points_[test]);
        const auto y = EuclideanMstDifference(points_[b], points_[test]);
        const auto z = EuclideanMstDifference(points_[c], points_[test]);
        // abの左にcがあるとき、行列式が正ならtestは外接円の内側。
        // 軸幅をWとすると絶対値は12W^4以下で、int64_tに収まる。
        return EuclideanMstNormSquared(x) * EuclideanMstCross(y, z) +
                   EuclideanMstNormSquared(y) * EuclideanMstCross(z, x) +
                   EuclideanMstNormSquared(z) * EuclideanMstCross(x, y) >
               0;
    }

    Index FindBase(Index &left, Index &right)
    {
        for (;;)
        {
            if (Side(left, Origin(right)) > 0)
            {
                left = AroundLeftFace(left);
            }
            else if (Side(right, Origin(left)) < 0)
            {
                right = AroundOrigin(Reverse(right));
            }
            else
            {
                return Connect(Reverse(right), left);
            }
        }
    }

    Index BorderCandidate(Index base, bool right)
    {
        const auto advance = [&](Index edge)
        {
            return right ? AroundOriginBack(edge) : AroundOrigin(edge);
        };
        auto candidate = advance(right ? base : Reverse(base));
        if (Side(base, Destination(candidate)) < 0)
        {
            auto next = advance(candidate);
            while (InCircle(Destination(base), Origin(base), Destination(candidate), Destination(next)))
            {
                Erase(candidate);
                candidate = next;
                next = advance(candidate);
            }
        }
        return candidate;
    }

    std::pair<Index, Index> Build(Index first, Index last)
    {
        const auto size = last - first;
        if (size <= 3)
        {
            const auto a = Add(first, first + 1);
            if (size == 2)
            {
                return {a, Reverse(a)};
            }
            const auto b = Add(first + 1, first + 2);
            Splice(Reverse(a), b);
            const auto turn = Side(a, first + 2);
            if (turn == 0)
            {
                return {a, Reverse(b)};
            }
            const auto close = Connect(b, a);
            if (turn > 0)
            {
                return {a, Reverse(b)};
            }
            return {Reverse(close), close};
        }

        const auto middle = first + size / 2;
        auto [outer_left, inner_left] = Build(first, middle);
        auto [inner_right, outer_right] = Build(middle, last);
        auto base = FindBase(inner_left, inner_right);
        if (Origin(inner_left) == Origin(outer_left))
        {
            outer_left = Reverse(base);
        }
        if (Origin(inner_right) == Origin(outer_right))
        {
            outer_right = base;
        }

        // 下側の共通接線から上へ進み、空円条件を満たす三角形で左右をつなぐ。
        for (;;)
        {
            const auto left = BorderCandidate(base, false);
            const auto right = BorderCandidate(base, true);
            const bool valid_left = Side(base, Destination(left)) < 0;
            const bool valid_right = Side(base, Destination(right)) < 0;
            if (!valid_left && !valid_right)
            {
                return {outer_left, outer_right};
            }
            if (!valid_left ||
                (valid_right && InCircle(Destination(left), Origin(left), Origin(right), Destination(right))))
            {
                base = Connect(right, Reverse(base));
            }
            else
            {
                base = Connect(Reverse(base), Reverse(left));
            }
        }
    }
};

inline void AddEuclideanMstEdges(std::span<const EuclideanMstPoint> points, std::span<const std::size_t> indices,
                                 std::vector<EuclideanMstEdge> &result)
{
    auto edges = EuclideanMstTriangulation(points).WeightedEdges();
    std::ranges::sort(edges, {}, &EuclideanMstEdge::distance_squared);

    std::vector<std::size_t> parent(points.size()), component_size(points.size(), 1);
    std::iota(parent.begin(), parent.end(), std::size_t{0});
    const auto find = [&](std::size_t v)
    {
        while (parent[v] != v)
        {
            parent[v] = parent[parent[v]];
            v = parent[v];
        }
        return v;
    };
    for (const auto &edge : edges)
    {
        auto u = find(edge.from), v = find(edge.to);
        if (u != v)
        {
            if (component_size[u] < component_size[v])
            {
                std::swap(u, v);
            }
            parent[v] = u;
            component_size[u] += component_size[v];
            result.push_back({indices[edge.from], indices[edge.to], edge.distance_squared});
        }
    }
}
} // namespace detail

inline std::vector<EuclideanMstEdge> euclidean_mst(std::span<const std::pair<std::int64_t, std::int64_t>> points)
{
    if (points.empty())
    {
        return {};
    }

    constexpr std::uint64_t kMaxSpan = 25'000;
    const auto min_x = std::ranges::min_element(points, {}, &detail::EuclideanMstPoint::first)->first;
    const auto min_y = std::ranges::min_element(points, {}, &detail::EuclideanMstPoint::second)->second;
    const auto translate = [&](std::int64_t coordinate, std::int64_t minimum)
    {
        const auto width = static_cast<std::uint64_t>(coordinate) - static_cast<std::uint64_t>(minimum);
        if (width > kMaxSpan)
        {
            throw std::overflow_error("euclidean_mst: coordinate range is too wide");
        }
        return static_cast<std::int64_t>(width);
    };

    std::vector<std::size_t> order(points.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    std::ranges::sort(order, {}, [&](std::size_t i)
                      {
                          return std::pair(points[i], i);
                      });

    std::vector<EuclideanMstEdge> result;
    std::vector<detail::EuclideanMstPoint> unique;
    std::vector<std::size_t> indices;
    result.reserve(points.size() - 1);
    unique.reserve(points.size());
    indices.reserve(points.size());
    for (const auto i : order)
    {
        const auto &[x, y] = points[i];
        const detail::EuclideanMstPoint point{translate(x, min_x), translate(y, min_y)};
        if (unique.empty() || unique.back() != point)
        {
            unique.push_back(point);
            indices.push_back(i);
        }
        else
        {
            result.push_back({indices.back(), i, 0});
        }
    }

    if (unique.size() >= 2)
    {
        detail::AddEuclideanMstEdges(unique, indices, result);
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
