// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Tree_B_Matching.cpp
// 参考（親辺を残す木DP）: https://atcoder.jp/contests/abc259/editorial/4296
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_b_matching.md
//
// API一覧
// maximum_weight_tree_b_matching(max_degree,edges): 各頂点の選択次数をmax_degree以下にして、辺重みの和を最大化。
// TreeBMatchingEdge{from,to,weight}: 頂点は0始まり、辺の重みは符号付き64ビット。
// TreeBMatchingResult{weight,edge_ids}: 最大値と、選んだ辺の入力添字。辺を選ばない解も許す。
// 非負の次数上限と無向木を渡す。空木・1頂点では重み0と空の辺列。最適値のint64_t超過はoverflow_error。
// 深い木でも非再帰。O(1+n log(n+1))時間・O(1+n)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_B_MATCHING_DEFINED
#define SIRKIT_TREE_B_MATCHING_DEFINED

struct TreeBMatchingEdge
{
    int from;
    int to;
    std::int64_t weight;
};

struct TreeBMatchingResult
{
    std::int64_t weight;
    std::vector<std::size_t> edge_ids;
};

namespace detail
{
struct TreeBMatchingArc
{
    int child;
    std::size_t edge;
};

struct TreeBMatchingGain
{
    std::int64_t value;
    TreeBMatchingArc arc;
};

using TreeBMatchingGraph = std::vector<std::vector<TreeBMatchingArc>>;

inline TreeBMatchingGraph tree_b_matching_graph(std::span<const int> max_degree,
                                                std::span<const TreeBMatchingEdge> edges)
{
    TreeBMatchingGraph graph(max_degree.size());
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &[from, to, weight] = edges[id];
        graph[from].push_back({to, id});
        graph[to].push_back({from, id});
    }
    return graph;
}

inline std::vector<int> tree_b_matching_order(const TreeBMatchingGraph &graph, std::span<int> parent)
{
    std::vector<int> order{0};
    parent[0] = -1;
    order.reserve(graph.size());
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        const int v = order[i];
        for (const auto &[to, id] : graph[v])
        {
            if (parent[to] == -2)
            {
                parent[to] = v;
                order.push_back(to);
            }
        }
    }
    return order;
}
} // namespace detail

inline TreeBMatchingResult maximum_weight_tree_b_matching(
    std::span<const int> max_degree, std::span<const TreeBMatchingEdge> edges)
{
    const auto n = max_degree.size();
    if (edges.size() != n - static_cast<std::size_t>(n != 0))
    {
        throw std::invalid_argument("maximum_weight_tree_b_matching: incorrect number of edges");
    }
    if (n == 0)
    {
        return {0, {}};
    }

    const auto graph = detail::tree_b_matching_graph(max_degree, edges);
    std::vector<int> parent(n, -2);
    const auto order = detail::tree_b_matching_order(graph, parent);
    std::vector<std::int64_t> penalty(n);
    std::vector<detail::TreeBMatchingGain> gains;
    std::vector<detail::TreeBMatchingArc> retained;
    retained.reserve(edges.size());
    std::vector<std::pair<std::size_t, std::size_t>> choice(n);
    TreeBMatchingResult result{0, {}};
    for (auto it = order.rbegin(); it != order.rend(); ++it)
    {
        const int v = *it;
        if (max_degree[v] == 0)
        {
            continue;
        }
        gains.clear();
        for (const auto &arc : graph[v])
        {
            const int u = arc.child;
            if (parent[u] != v || max_degree[u] == 0)
            {
                continue;
            }
            // 親辺に枠を使うとき、子側で諦める利得は子の最後の採用分だけ。
            const auto weight = edges[arc.edge].weight;
            if (weight > penalty[u])
            {
                gains.push_back({weight - penalty[u], arc});
            }
        }
        std::sort(gains.begin(), gains.end(), [](const auto &a, const auto &b)
                  {
                      if (a.value != b.value)
                      {
                          return a.value > b.value;
                      }
                      return a.arc.edge < b.arc.edge;
                  });

        const auto limit = static_cast<std::size_t>(max_degree[v]);
        const auto take = std::min(limit, gains.size());
        choice[v] = {retained.size(), take};
        if (take == limit)
        {
            penalty[v] = gains[take - 1].value;
        }
        for (std::size_t i = 0; i < take; ++i)
        {
            // 全頂点の採用利得の和は、根の木DPと等しい。後続の利得も非負。
            if (gains[i].value > std::numeric_limits<std::int64_t>::max() - result.weight)
            {
                throw std::overflow_error("maximum_weight_tree_b_matching: optimal weight does not fit int64_t");
            }
            result.weight += gains[i].value;
            retained.push_back(gains[i].arc);
        }
    }

    std::vector<bool> parent_selected(n);
    result.edge_ids.reserve(retained.size());
    for (const int v : order)
    {
        const auto [first, take] = choice[v];
        const auto loss = parent_selected[v] && take == static_cast<std::size_t>(max_degree[v]);
        for (std::size_t i = 0; i < take - static_cast<std::size_t>(loss); ++i)
        {
            const auto [u, id] = retained[first + i];
            parent_selected[u] = true;
            result.edge_ids.push_back(id);
        }
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
