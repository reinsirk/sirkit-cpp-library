// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Chordal_Graph.cpp
// 参考（最大基数探索）: https://doi.org/10.1137/0213035
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/chordal_graph.md
//
// API一覧
// recognize_chordal_graph(n,edges): 弦グラフ判定と証明を返す。頂点番号は[0,n)。
// ChordalGraphResult: is_chordalがtrueならperfect_elimination_orderに全頂点を格納する。
// falseならinduced_cycleに4頂点以上の誘導閉路を格納する。始点の再掲はしない。
// 自己辺を無視し、重複辺はまとめて扱う。孤立頂点と空グラフも可。O(n+m)時間・領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CHORDAL_GRAPH_DEFINED
#define SIRKIT_CHORDAL_GRAPH_DEFINED

struct ChordalGraphResult
{
    bool is_chordal;
    std::vector<int> perfect_elimination_order;
    std::vector<int> induced_cycle;
};

namespace detail
{
struct ChordalAdjacency
{
    std::vector<std::size_t> offsets;
    std::vector<int> targets;

    std::span<const int> neighbors(int vertex) const
    {
        return std::span<const int>(targets).subspan(offsets[vertex], offsets[vertex + 1] - offsets[vertex]);
    }
};

inline ChordalAdjacency make_chordal_adjacency(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("recognize_chordal_graph: negative vertex count");
    }
    ChordalAdjacency graph{std::vector<std::size_t>(static_cast<std::size_t>(n) + 1), {}};
    for (const auto &[u, v] : edges)
    {
        if (u != v)
        {
            ++graph.offsets[u + 1];
            ++graph.offsets[v + 1];
        }
    }
    for (int v = 0; v < n; ++v)
    {
        graph.offsets[v + 1] += graph.offsets[v];
    }
    graph.targets.resize(graph.offsets[n]);
    auto cursor = graph.offsets;
    for (const auto &[u, v] : edges)
    {
        if (u != v)
        {
            graph.targets[cursor[u]++] = v;
            graph.targets[cursor[v]++] = u;
        }
    }
    return graph;
}

struct ChordalSearchOrder
{
    std::vector<int> order;
    std::vector<int> rank;
    // MCS順で直近の選択済み隣接頂点。
    std::vector<int> parent;
};

inline ChordalSearchOrder chordal_maximum_cardinality_search(int n, const ChordalAdjacency &graph)
{
    constexpr auto kEmpty = std::numeric_limits<std::size_t>::max();
    struct BucketEntry
    {
        std::size_t next;
        int vertex;
    };

    std::vector<std::size_t> heads(n, kEmpty);
    std::vector<BucketEntry> entries;
    entries.reserve(static_cast<std::size_t>(n) + graph.targets.size() / 2);
    const auto enqueue = [&](int label, int v)
    {
        entries.push_back({heads[label], v});
        heads[label] = entries.size() - 1;
    };
    for (int v = 0; v < n; ++v)
    {
        enqueue(0, v);
    }

    ChordalSearchOrder search{{}, std::vector<int>(n), std::vector<int>(n, -1)};
    search.order.reserve(n);
    std::vector<int> label(n);
    int largest = 0;
    for (int i = 0; i < n; ++i)
    {
        int vertex;
        // 更新前のバケット要素は、取り出したときに捨てる。
        for (;;)
        {
            auto &head = heads[largest];
            if (head == kEmpty)
            {
                --largest;
                continue;
            }
            const auto entry = entries[head];
            head = entry.next;
            if (label[entry.vertex] == largest)
            {
                vertex = entry.vertex;
                break;
            }
        }
        label[vertex] = -1;
        search.rank[vertex] = i;
        search.order.push_back(vertex);
        for (int to : graph.neighbors(vertex))
        {
            if (label[to] >= 0 && search.parent[to] != vertex)
            {
                // 同じ頂点から来た多重辺では、ラベルを一度だけ増やす。
                search.parent[to] = vertex;
                ++label[to];
                enqueue(label[to], to);
                largest = std::max(largest, label[to]);
            }
        }
    }
    return search;
}

inline int first_chordal_elimination_violation(const ChordalAdjacency &graph, const ChordalSearchOrder &search)
{
    const int n = static_cast<int>(search.order.size());
    std::vector<int> first_child(n, -1), sibling(n, -1);
    for (int v = 0; v < n; ++v)
    {
        if (const int parent = search.parent[v]; parent >= 0)
        {
            sibling[v] = first_child[parent];
            first_child[parent] = v;
        }
    }
    std::vector<int> adjacent(n, -1);
    int earliest = n;
    for (int parent : search.order)
    {
        for (int to : graph.neighbors(parent))
        {
            adjacent[to] = parent;
        }
        for (int child = first_child[parent]; child >= 0; child = sibling[child])
        {
            for (int to : graph.neighbors(child))
            {
                if (search.rank[to] < search.rank[parent] && adjacent[to] != parent)
                {
                    earliest = std::min(earliest, search.rank[child]);
                    break;
                }
            }
        }
    }
    return earliest;
}

inline std::vector<int> chordal_cycle_from_boundary(
    const ChordalAdjacency &graph, const std::vector<int> &component, int seed, int apex, int first, int last)
{
    std::vector<int> predecessor(component.size(), -1);
    predecessor[first] = first;
    std::vector<int> queue{first};
    // firstとlastはどちらも成分seedに接するので、成分内を通って到達できる。
    for (std::size_t head = 0; predecessor[last] < 0; ++head)
    {
        const int from = queue[head];
        for (int to : graph.neighbors(from))
        {
            if (predecessor[to] < 0 && (component[to] == seed || to == last))
            {
                predecessor[to] = from;
                queue.push_back(to);
            }
        }
    }
    std::vector<int> cycle{apex};
    for (int v = last; v != first; v = predecessor[v])
    {
        cycle.push_back(v);
    }
    cycle.push_back(first);
    return cycle;
}

inline std::vector<int> chordal_prefix_hole(
    const ChordalAdjacency &graph, const ChordalSearchOrder &search, int cutoff)
{
    constexpr int kUnseen = -1;
    constexpr int kNeighbor = -2;
    struct Boundary
    {
        std::size_t begin;
        std::size_t end;
        int component;
        int next;
    };

    const auto n = search.order.size();
    const int apex = search.order[cutoff];
    std::vector<int> component(n, kUnseen), stamp(n, kUnseen), group(n, kUnseen);
    for (int v : graph.neighbors(apex))
    {
        component[v] = kNeighbor;
    }

    std::vector<int> boundary_vertices;
    std::vector<Boundary> boundaries;
    std::vector<int> queue;
    for (int i = 0; i < cutoff; ++i)
    {
        const int seed = search.order[i];
        if (component[seed] != kUnseen)
        {
            continue;
        }
        component[seed] = seed;
        queue.clear();
        queue.push_back(seed);
        const auto begin = boundary_vertices.size();
        int pivot = -1;
        for (std::size_t head = 0; head < queue.size(); ++head)
        {
            for (int to : graph.neighbors(queue[head]))
            {
                if (search.rank[to] >= cutoff)
                {
                    continue;
                }
                if (component[to] == kNeighbor)
                {
                    if (stamp[to] != seed)
                    {
                        stamp[to] = seed;
                        boundary_vertices.push_back(to);
                        if (pivot < 0 || search.rank[pivot] < search.rank[to])
                        {
                            pivot = to;
                        }
                    }
                }
                else if (component[to] == kUnseen)
                {
                    component[to] = seed;
                    queue.push_back(to);
                }
            }
        }
        if (pivot >= 0)
        {
            boundaries.push_back({begin, boundary_vertices.size(), seed, group[pivot]});
            group[pivot] = static_cast<int>(boundaries.size() - 1);
        }
    }

    // 頂点apexより前の部分は弦グラフ。境界の先頭を除去する段階で、
    // 残りが先頭にすべて隣接すれば完全除去順の性質から境界はクリーク。
    std::fill(stamp.begin(), stamp.end(), kUnseen);
    for (int i = 0; i < cutoff; ++i)
    {
        const int pivot = search.order[i];
        if (group[pivot] < 0)
        {
            continue;
        }
        for (int to : graph.neighbors(pivot))
        {
            stamp[to] = pivot;
        }
        for (int b = group[pivot]; b >= 0; b = boundaries[b].next)
        {
            const auto &boundary = boundaries[b];
            for (auto j = boundary.begin; j < boundary.end; ++j)
            {
                const int v = boundary_vertices[j];
                if (v != pivot && stamp[v] != pivot)
                {
                    return chordal_cycle_from_boundary(graph, component, boundary.component, apex, pivot, v);
                }
            }
        }
    }
    return {};
}
} // namespace detail

inline ChordalGraphResult recognize_chordal_graph(int n, std::span<const std::pair<int, int>> edges)
{
    const auto graph = detail::make_chordal_adjacency(n, edges);
    auto search = detail::chordal_maximum_cardinality_search(n, graph);
    const int cutoff = detail::first_chordal_elimination_violation(graph, search);
    if (cutoff < n)
    {
        // MCSの先頭cutoff+1頂点で初めて弦グラフでなくなる。
        // その部分グラフの誘導閉路はcutoff番目の頂点を必ず通る。
        return {false, {}, detail::chordal_prefix_hole(graph, search, cutoff)};
    }
    std::reverse(search.order.begin(), search.order.end());
    return {true, std::move(search.order), {}};
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
