// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/ST_Numbering.cpp
// 参考（Tarjanの符号挿入法）: https://doi.org/10.3233/FI-1986-9105
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/st_numbering.md
//
// API一覧
// st_numbering(n,source,sink,edges): 無向グラフのST番号付けを返す。存在しなければstd::nullopt。
// rank[v]が頂点vの0始まりの順位。sourceは0、sinkはn-1。各辺を小さい順位から大きい順位へ向けると
// 各頂点がsourceからsinkへ向かうパスに載る。sourceとsinkは非隣接でも可。
// 頂点番号は[0,n)。多重辺可、自己辺は無視する。空グラフ・同一端点には番号付けなし。
// 反復DFSと連結リストでO(n+m)時間・領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ST_NUMBERING_DEFINED
#define SIRKIT_ST_NUMBERING_DEFINED

namespace detail
{
struct StNumberingAdjacency
{
    std::vector<std::size_t> offsets;
    std::vector<int> neighbors;
};

inline StNumberingAdjacency make_st_numbering_graph(int n, std::span<const std::pair<int, int>> edges)
{
    StNumberingAdjacency graph{std::vector<std::size_t>(static_cast<std::size_t>(n) + 1), {}};
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
    graph.neighbors.resize(graph.offsets.back());
    auto cursor = graph.offsets;
    for (const auto &[u, v] : edges)
    {
        if (u != v)
        {
            graph.neighbors[cursor[u]++] = v;
            graph.neighbors[cursor[v]++] = u;
        }
    }
    return graph;
}

inline std::vector<int> restore_st_ranks(
    const std::vector<int> &preorder, const std::vector<int> &parent, const std::vector<int> &low)
{
    const auto n = preorder.size();
    const int source = preorder[0], sink = preorder[1];
    std::vector<int> link_before(n, -1), link_after(n, -1);
    std::vector<unsigned char> positive(n);
    link_before[sink] = source;
    link_after[source] = sink;
    for (std::size_t i = 2; i < n; ++i)
    {
        const int v = preorder[i], p = parent[v];
        const bool insert_after = positive[preorder[low[v]]] != 0;
        const int before = insert_after ? p : link_before[p];
        const int after = insert_after ? link_after[p] : p;
        link_after[before] = v;
        link_before[after] = v;
        link_before[v] = before;
        link_after[v] = after;
        positive[p] = !insert_after;
    }

    std::vector<int> ranks(n);
    int rank = 0;
    for (int v = source; v >= 0; v = link_after[v])
    {
        ranks[v] = rank++;
    }
    return ranks;
}
} // namespace detail

inline std::optional<std::vector<int>> st_numbering(
    int n, int source, int sink, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("st_numbering: negative vertex count");
    }
    if (n == 0)
    {
        return std::nullopt;
    }
    if (source == sink || edges.empty())
    {
        return std::nullopt;
    }
    const auto graph = detail::make_st_numbering_graph(n, edges);
    if (graph.offsets[source] == graph.offsets[source + 1])
    {
        return std::nullopt;
    }

    struct Frame
    {
        int vertex;
        std::size_t next;
    };
    std::vector<Frame> pending;
    std::vector<int> parent(n), number(n, -1), low(n), preorder;
    preorder.reserve(n);
    const auto enter = [&](int v, int p)
    {
        parent[v] = p;
        number[v] = low[v] = static_cast<int>(preorder.size());
        preorder.push_back(v);
        pending.push_back({v, graph.offsets[v]});
    };

    // 仮のsource-sink辺を最初に通ったとみなし、source以外をsinkから探索する。
    number[source] = 0;
    preorder.push_back(source);
    enter(sink, source);
    while (!pending.empty())
    {
        auto &frame = pending.back();
        const int v = frame.vertex;
        if (frame.next != graph.offsets[v + 1])
        {
            const int to = graph.neighbors[frame.next++];
            if (number[to] < 0)
            {
                enter(to, v);
            }
            else
            {
                // 親への辺が混ざっても、下の「親より厳密に小さい」条件には影響しない。
                low[v] = std::min(low[v], number[to]);
            }
            continue;
        }

        pending.pop_back();
        if (v != sink)
        {
            const int p = parent[v];
            if (low[v] >= number[p])
            {
                return std::nullopt;
            }
            low[p] = std::min(low[p], low[v]);
        }
    }
    if (preorder.size() != static_cast<std::size_t>(n))
    {
        return std::nullopt;
    }
    return detail::restore_st_ranks(preorder, parent, low);
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
