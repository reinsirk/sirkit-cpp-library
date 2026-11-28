// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Bipartite_Coloring.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bipartite_coloring.md
//
// API一覧
// bipartite_coloring(n, edges): 無向グラフを二部に分ける。頂点0..n-1。自己辺・多重辺可。
//   二部グラフなら color[v] は 0 または 1、odd_cycle は nullopt。
//   二部グラフでなければ color は空、odd_cycle に頂点列と入力の辺番号列を返す。
//   O(n+m)時間・領域。
// BipartiteColoring::is_bipartite(): 2色に分けられる場合にtrue。O(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
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

#ifndef SIRKIT_BIPARTITE_COLORING_DEFINED
#define SIRKIT_BIPARTITE_COLORING_DEFINED

struct BipartiteColoring
{
    std::vector<std::int8_t> color;
    std::optional<Cycle> odd_cycle;

    [[nodiscard]] bool is_bipartite() const noexcept
    {
        return !odd_cycle;
    }
};

inline BipartiteColoring bipartite_coloring(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("bipartite_coloring: negative size");
    }
    struct Arc
    {
        int to;
        std::size_t id;
    };
    std::vector<std::vector<Arc>> graph(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("bipartite_coloring: invalid endpoint");
        }
#endif
        graph[u].push_back({v, id});
        graph[v].push_back({u, id});
    }

    std::vector<std::int8_t> color(n, -1);
    std::vector<int> parent(n, -1);
    std::vector<std::size_t> parent_edge(n);
    std::vector<int> pending;
    pending.reserve(n);
    for (int root = 0; root < n; ++root)
    {
        if (color[root] != -1)
        {
            continue;
        }
        color[root] = 0;
        pending.clear();
        pending.push_back(root);
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            const int u = pending[i];
            for (const auto &[v, id] : graph[u])
            {
                if (color[v] == -1)
                {
                    color[v] = static_cast<std::int8_t>(1 - color[u]);
                    parent[v] = u;
                    parent_edge[v] = id;
                    pending.push_back(v);
                }
                else if (color[v] == color[u])
                {
                    Cycle left, right;
                    int a = u;
                    int b = v;
                    // Adjacent vertices of the same color are at the same BFS depth.
                    while (a != b)
                    {
                        left.vertices.push_back(a);
                        left.edge_ids.push_back(parent_edge[a]);
                        right.vertices.push_back(b);
                        right.edge_ids.push_back(parent_edge[b]);
                        a = parent[a];
                        b = parent[b];
                    }
                    left.vertices.push_back(a);
                    std::ranges::reverse(left.vertices);
                    std::ranges::reverse(left.edge_ids);
                    left.edge_ids.push_back(id);
                    left.vertices.insert(left.vertices.end(), right.vertices.begin(), right.vertices.end());
                    left.edge_ids.insert(left.edge_ids.end(), right.edge_ids.begin(), right.edge_ids.end());
                    return {{}, std::move(left)};
                }
            }
        }
    }
    return {std::move(color), std::nullopt};
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
