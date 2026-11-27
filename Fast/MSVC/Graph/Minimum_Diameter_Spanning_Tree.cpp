// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Minimum_Diameter_Spanning_Tree.cpp
// 参考（絶対中心と最小直径全域木）: https://doi.org/10.1016/0020-0190(94)00183-Y
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/minimum_diameter_spanning_tree.md
//
// API一覧
// MinimumDiameterTreeEdge: 無向辺{from,to,weight}。頂点は[0,n)、重みは非負のint64_t。
// minimum_diameter_spanning_tree(n,edges): 最小直径全域木を{diameter,edge_ids}で返す。
// edge_idsは選んだ辺の入力添字。多重辺・重み0を扱い、自己辺は無視する。
// 空または非連結ならnullopt、1頂点では直径0、最適直径がint64_tの範囲外ならoverflow_error。
// O(n²+nm log(n+1))時間・O(n²+m)領域。頂点数は距離行列を確保できる範囲。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MINIMUM_DIAMETER_SPANNING_TREE_DEFINED
#define SIRKIT_MINIMUM_DIAMETER_SPANNING_TREE_DEFINED

struct MinimumDiameterTreeEdge
{
    int from;
    int to;
    std::int64_t weight;
};

struct MinimumDiameterTreeResult
{
    std::int64_t diameter;
    std::vector<std::size_t> edge_ids;
};

namespace detail
{
using DiameterCost = std::uint64_t;
inline constexpr DiameterCost kDiameterOverflow = DiameterCost{1} << 63;
inline constexpr DiameterCost kDiameterUnreachable = std::numeric_limits<DiameterCost>::max();
inline constexpr std::size_t kDiameterNoEdge = std::numeric_limits<std::size_t>::max();

inline DiameterCost capped_diameter_add(DiameterCost a, DiameterCost b)
{
    return a >= kDiameterOverflow || b >= kDiameterOverflow - a ? kDiameterOverflow : a + b;
}

struct DiameterArc
{
    int to;
    std::size_t edge_id;
};

class DiameterHeap
{
    std::span<const DiameterCost> distance_;
    std::vector<int> vertices_;
    std::vector<std::size_t> position_;

    void put(std::size_t i, int v)
    {
        vertices_[i] = v;
        position_[v] = i;
    }

public:
    explicit DiameterHeap(std::size_t n) : position_(n, kDiameterNoEdge)
    {
        vertices_.reserve(n);
    }

    void reset(std::span<const DiameterCost> distance)
    {
        distance_ = distance;
        vertices_.clear();
        std::fill(position_.begin(), position_.end(), kDiameterNoEdge);
    }

    bool empty() const
    {
        return vertices_.empty();
    }

    void decrease(int v)
    {
        auto i = position_[v];
        if (i == kDiameterNoEdge)
        {
            i = vertices_.size();
            vertices_.push_back(v);
        }
        while (i != 0)
        {
            const auto parent = (i - 1) / 2;
            const int previous = vertices_[parent];
            if (distance_[previous] <= distance_[v])
            {
                break;
            }
            put(i, previous);
            i = parent;
        }
        put(i, v);
    }

    int pop()
    {
        const int top = vertices_.front(), last = vertices_.back();
        vertices_.pop_back();
        position_[top] = kDiameterNoEdge;
        if (!vertices_.empty())
        {
            std::size_t i = 0;
            for (std::size_t child = 1; child < vertices_.size(); child = i * 2 + 1)
            {
                const auto right = child + 1;
                if (right < vertices_.size() && distance_[vertices_[right]] < distance_[vertices_[child]])
                {
                    child = right;
                }
                const int next = vertices_[child];
                if (distance_[next] >= distance_[last])
                {
                    break;
                }
                put(i, next);
                i = child;
            }
            put(i, last);
        }
        return top;
    }
};

struct DiameterSeed
{
    int vertex;
    DiameterCost distance;
};

inline std::size_t diameter_dijkstra(
    const std::vector<std::vector<DiameterArc>> &graph, std::span<const MinimumDiameterTreeEdge> edges,
    DiameterHeap &heap, std::span<DiameterCost> distance, DiameterSeed first, DiameterSeed second,
    unsigned scale, std::span<int> order, std::span<std::size_t> parent)
{
    std::fill(distance.begin(), distance.end(), kDiameterUnreachable);
    std::fill(parent.begin(), parent.end(), kDiameterNoEdge);
    heap.reset(distance);
    distance[first.vertex] = first.distance;
    heap.decrease(first.vertex);
    if (second.vertex >= 0)
    {
        distance[second.vertex] = second.distance;
        heap.decrease(second.vertex);
    }
    std::size_t count = 0;
    while (!heap.empty())
    {
        const int from = heap.pop();
        if (!order.empty())
        {
            order[count] = from;
        }
        ++count;
        for (const auto &[to, edge_id] : graph[from])
        {
            const auto weight = static_cast<DiameterCost>(edges[edge_id].weight) * scale;
            const auto next = capped_diameter_add(distance[from], weight);
            if (next < distance[to])
            {
                distance[to] = next;
                if (!parent.empty())
                {
                    parent[to] = edge_id;
                }
                heap.decrease(to);
            }
        }
    }
    return count;
}

struct DiameterCenter
{
    DiameterCost diameter = kDiameterOverflow;
    int vertex = -1;
    std::size_t edge = kDiameterNoEdge;
    DiameterCost position_twice = 0;
};

inline void consider_diameter_edge(
    DiameterCenter &best, std::size_t id, DiameterCost weight, std::span<const DiameterCost> from,
    std::span<const DiameterCost> to, std::span<const int> order)
{
    auto right_max = to[order.back()];
    for (std::size_t i = order.size() - 1; i != 0; --i)
    {
        const int v = order[i - 1];
        if (to[v] <= right_max)
        {
            continue;
        }
        // 片端距離の降順で、もう片端距離が伸びる点だけが必要。
        // 隣接する候補の切替点間では、右からの直線と左からの直線が交わる点が最良。
        const auto right = weight + right_max;
        const auto diameter = capped_diameter_add(from[v], right);
        if (diameter < best.diameter)
        {
            best = {diameter, -1, id, right - from[v]};
        }
        right_max = to[v];
    }
}
} // namespace detail

inline std::optional<MinimumDiameterTreeResult> minimum_diameter_spanning_tree(
    int n, std::span<const MinimumDiameterTreeEdge> edges)
{
    if (n < 0)
    {
        throw std::length_error("minimum_diameter_spanning_tree: negative vertex count");
    }
    if (n == 0)
    {
        return std::nullopt;
    }
    const auto count = static_cast<std::size_t>(n);
    using Cost = detail::DiameterCost;
    constexpr auto kMaxCells = static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / sizeof(Cost);
    if (count > kMaxCells / count)
    {
        throw std::length_error("minimum_diameter_spanning_tree: distance matrix too large");
    }

    std::vector<std::vector<detail::DiameterArc>> graph(count);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &[from, to, weight] = edges[id];
        if (from != to)
        {
            graph[from].push_back({to, id});
            graph[to].push_back({from, id});
        }
    }
    detail::DiameterHeap heap(count);
    std::vector<Cost> distances(count * count);
    std::vector<int> orders(count * count);
    const auto row = [&](int v)
    {
        return std::span<Cost>(distances).subspan(static_cast<std::size_t>(v) * count, count);
    };
    const auto order = [&](int v)
    {
        return std::span<int>(orders).subspan(static_cast<std::size_t>(v) * count, count);
    };

    detail::DiameterCenter best;
    for (int v = 0; v < n; ++v)
    {
        const auto distance = row(v);
        const auto settled = order(v);
        const auto reached = detail::diameter_dijkstra(graph, edges, heap, distance, {v, 0}, {-1, 0}, 1, settled, {});
        if (reached != count)
        {
            return std::nullopt;
        }
        const auto radius = distance[settled.back()];
        if (radius == detail::kDiameterOverflow)
        {
            throw std::overflow_error("minimum_diameter_spanning_tree: graph distance does not fit int64_t");
        }
        const auto diameter = detail::capped_diameter_add(radius, radius);
        if (diameter < best.diameter)
        {
            best = {diameter, v, detail::kDiameterNoEdge, 0};
        }
    }
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &[from, to, weight] = edges[id];
        if (from != to && weight != 0)
        {
            detail::consider_diameter_edge(best, id, static_cast<Cost>(weight), row(from), row(to), order(from));
        }
    }
    if (best.diameter == detail::kDiameterOverflow)
    {
        throw std::overflow_error("minimum_diameter_spanning_tree: minimum diameter does not fit int64_t");
    }

    std::vector<std::size_t> parent(count);
    MinimumDiameterTreeResult result{static_cast<std::int64_t>(best.diameter), {}};
    result.edge_ids.reserve(count - 1);
    if (best.edge == detail::kDiameterNoEdge)
    {
        detail::diameter_dijkstra(graph, edges, heap, row(0), {best.vertex, 0}, {-1, 0}, 1, {}, parent);
    }
    else
    {
        // 端点の中心より厳密に良いときだけこの経路に来るため、両端が仮想中心に直接つながる。
        const auto &[from, to, weight] = edges[best.edge];
        detail::diameter_dijkstra(graph, edges, heap, row(0), {from, best.position_twice},
                                  {to, 2 * static_cast<Cost>(weight) - best.position_twice}, 2, {}, parent);
        result.edge_ids.push_back(best.edge);
    }
    for (const auto id : parent)
    {
        if (id != detail::kDiameterNoEdge)
        {
            result.edge_ids.push_back(id);
        }
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
