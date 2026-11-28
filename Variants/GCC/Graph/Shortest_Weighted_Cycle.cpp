// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Shortest_Weighted_Cycle.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/shortest_weighted_cycle.md
//
// API一覧
// shortest_weighted_cycle(n, edges, directed=true, through=std::nullopt):
//   重みの和が最小の単純閉路を返す。throughを指定するとその頂点を含む閉路だけを調べる。
//   WeightedCycleEdge{from,to,weight=1}。重みは非負のuint64_t。自己辺と多重辺も扱う。
//   閉路がないか、重みの和がuint64_tの上限を超えるとnullopt。
//   1始点: O((n+m)log(n+m+1))時間。全頂点: O(n(n+m)log(n+m+1))時間。領域O(n+m+1)。
// WeightedCycle{weight,cycle}: 重みの和と、巡回順の頂点列・入力の辺番号列。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <numeric>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CYCLE_DETECTION_DEFINED
#define SIRKIT_CYCLE_DETECTION_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t cycle_arc_count(std::size_t edges, std::size_t directions, std::size_t limit)
{
    if (edges > limit / directions)
    {
        throw std::length_error("find_cycle: too many edges");
    }
    return edges * directions;
}
} // namespace detail

struct Cycle
{
    std::vector<int> vertices;
    std::vector<std::size_t> edge_ids;
};

inline std::optional<Cycle> find_cycle(int n, std::span<const std::pair<int, int>> edges, bool directed = true)
{
    if (n < 0)
    {
        throw std::length_error("find_cycle: negative vertex count");
    }
    struct Arc
    {
        int to;
        std::size_t id;
    };
    const auto directions = directed ? 1U : 2U;
    const auto arc_count = detail::cycle_arc_count(edges.size(), directions, std::vector<Arc>{}.max_size());

    std::vector<std::size_t> offset(static_cast<std::size_t>(n) + 1);
    for (const auto &[u, v] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("find_cycle: vertex out of range");
        }
#endif
        ++offset[u];
        if (!directed)
        {
            ++offset[v];
        }
    }
    std::partial_sum(offset.begin(), offset.end(), offset.begin());
    std::vector<Arc> adjacency(arc_count);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v] = edges[id];
        adjacency[--offset[u]] = {v, id};
        if (!directed)
        {
            adjacency[--offset[v]] = {u, id};
        }
    }

    struct Frame
    {
        int vertex;
        std::size_t next;
        std::size_t incoming;
    };
    constexpr int kUnseen = -1;
    constexpr int kFinished = -2;
    std::vector<int> position(n, kUnseen);
    std::vector<Frame> stack;
    for (int start = 0; start < n; ++start)
    {
        if (position[start] != kUnseen)
        {
            continue;
        }
        position[start] = 0;
        stack.push_back({start, offset[start], edges.size()});
        while (!stack.empty())
        {
            auto &frame = stack.back();
            if (frame.next == offset[frame.vertex + 1])
            {
                position[frame.vertex] = kFinished;
                stack.pop_back();
                continue;
            }
            const auto [to, id] = adjacency[frame.next++];
            if (!directed && id == frame.incoming)
            {
                continue;
            }
            if (position[to] == kUnseen)
            {
                position[to] = static_cast<int>(stack.size());
                stack.push_back({to, offset[to], id});
            }
            else if (position[to] >= 0)
            {
                const auto first = static_cast<std::size_t>(position[to]);
                Cycle result;
                result.vertices.reserve(stack.size() - first);
                result.edge_ids.reserve(stack.size() - first);
                result.vertices.push_back(to);
                for (std::size_t i = first + 1; i < stack.size(); ++i)
                {
                    result.vertices.push_back(stack[i].vertex);
                    result.edge_ids.push_back(stack[i].incoming);
                }
                result.edge_ids.push_back(id);
                return result;
            }
        }
    }
    return std::nullopt;
}

#endif

#ifndef SIRKIT_SHORTEST_WEIGHTED_CYCLE_DEFINED
#define SIRKIT_SHORTEST_WEIGHTED_CYCLE_DEFINED

struct WeightedCycleEdge
{
    int from;
    int to;
    std::uint64_t weight = 1;
};

struct WeightedCycle
{
    std::uint64_t weight;
    Cycle cycle;
};

namespace detail
{
struct WeightedCycleArc
{
    int to;
    std::size_t id;
    std::uint64_t weight;
};

using WeightedCycleGraph = std::vector<std::vector<WeightedCycleArc>>;

inline std::optional<WeightedCycle> shortest_weighted_cycle_from(
    const WeightedCycleGraph &graph, int source, std::size_t edge_count, bool directed,
    std::optional<std::uint64_t> limit)
{
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    struct Connection
    {
        int from;
        int to;
        std::size_t id;
        std::uint64_t cost;
    };
    std::optional<Connection> best;
    auto consider = [&](int from, int to, std::size_t id, std::uint64_t cost)
    {
        if (!limit || cost < *limit)
        {
            best = {from, to, id, cost};
            limit = cost;
        }
    };

    std::vector<std::optional<std::uint64_t>> distance(graph.size());
    std::vector<int> parent(graph.size(), -1);
    std::vector<std::size_t> incoming(graph.size());
    std::vector<std::size_t> first(graph.size(), edge_count);
    std::vector<bool> settled(graph.size());
    using State = std::pair<std::uint64_t, int>;
    std::priority_queue<State, std::vector<State>, std::greater<State>> queue;
    distance[source] = 0;
    queue.emplace(0, source);

    while (!queue.empty())
    {
        const auto [prefix, from] = queue.top();
        queue.pop();
        if (prefix != distance[from].value())
        {
            continue;
        }
        if (limit && prefix >= *limit)
        {
            break;
        }
        settled[from] = true;
        for (const auto &[to, id, weight] : graph[from])
        {
            if (weight > kMax - prefix)
            {
                continue;
            }
            const auto candidate = prefix + weight;
            if (to == source)
            {
                if (directed || first[from] != id)
                {
                    consider(from, to, id, candidate);
                }
                continue;
            }
            // In an undirected graph, two different first edges identify disjoint tree paths from source.
            if (!directed && from != source && settled[to] && first[from] != first[to])
            {
                const auto suffix = distance[to].value();
                if (suffix <= kMax - candidate)
                {
                    consider(from, to, id, candidate + suffix);
                }
            }
            if (!distance[to] || candidate < distance[to].value())
            {
                distance[to] = candidate;
                parent[to] = from;
                incoming[to] = id;
                first[to] = from == source ? id : first[from];
                queue.emplace(candidate, to);
            }
        }
    }
    if (!best)
    {
        return std::nullopt;
    }

    WeightedCycle result{best->cost, {}};
    auto &vertices = result.cycle.vertices;
    auto &edges = result.cycle.edge_ids;
    for (int v = best->from; v != source; v = parent[v])
    {
        vertices.push_back(v);
        edges.push_back(incoming[v]);
    }
    vertices.push_back(source);
    std::ranges::reverse(vertices);
    std::ranges::reverse(edges);
    edges.push_back(best->id);
    for (int v = best->to; v != source; v = parent[v])
    {
        vertices.push_back(v);
        edges.push_back(incoming[v]);
    }
    return result;
}
} // namespace detail

inline std::optional<WeightedCycle> shortest_weighted_cycle(
    int n, std::span<const WeightedCycleEdge> edges, bool directed = true,
    std::optional<int> through = std::nullopt)
{
    if (n < 0)
    {
        throw std::length_error("shortest_weighted_cycle: negative vertex count");
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (through && (*through < 0 || *through >= n))
    {
        throw std::out_of_range("shortest_weighted_cycle: invalid required vertex");
    }
#endif
    detail::WeightedCycleGraph graph(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [from, to, weight] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("shortest_weighted_cycle: invalid endpoint");
        }
#endif
        graph[from].push_back({to, id, weight});
        if (!directed)
        {
            graph[to].push_back({from, id, weight});
        }
    }

    std::optional<WeightedCycle> result;
    const int first = through.value_or(0);
    const int last = through ? *through + 1 : n;
    for (int v = first; v < last; ++v)
    {
        if (graph[v].empty())
        {
            continue;
        }
        const auto bound = result ? std::optional<std::uint64_t>(result->weight) : std::nullopt;
        if (auto better = detail::shortest_weighted_cycle_from(graph, v, edges.size(), directed, bound))
        {
            result = std::move(better);
            if (result->weight == 0)
            {
                break;
            }
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
