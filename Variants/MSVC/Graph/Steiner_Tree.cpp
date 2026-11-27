// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Graph/Steiner_Tree.cpp
// 参考（部分集合DP）: https://doi.org/10.1002/net.3230010302
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/steiner_tree.md
//
// API一覧
// SteinerTreeEdge{from,to,weight}: 端点が[0,n)の無向辺。重みは非負のint64_t。
// minimum_steiner_tree(n,edges,terminals): 全指定頂点を結ぶ最小費用の木を返す。
// SteinerTree{cost,edge_ids}: 費用と、選んだ辺の入力位置。順序は任意。
// 指定頂点の重複を除き、0個または1個なら費用0と空の辺列。連結不能ならnullopt。
// 費用がint64_tの上限を超えるとoverflow_error。自己辺・多重辺可。
// 異なる指定頂点がk≥2個ならb=k−1、Tを重複込みの指定数として、時間O(T+3^b*n+2^b*(n+m)*log(n+m+1))、領域O(2^b*n+m)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STEINER_TREE_DEFINED
#define SIRKIT_STEINER_TREE_DEFINED

struct SteinerTreeEdge
{
    int from;
    int to;
    std::int64_t weight;
};

struct SteinerTree
{
    std::int64_t cost;
    std::vector<std::size_t> edge_ids;
};

inline std::optional<SteinerTree> minimum_steiner_tree(
    int n, std::span<const SteinerTreeEdge> edges, std::span<const int> terminals)
{
    if (n < 0)
    {
        throw std::length_error("minimum_steiner_tree: negative vertex count");
    }
    if (terminals.empty())
    {
        return SteinerTree{0, {}};
    }
    const auto vertex_count = static_cast<std::size_t>(n);
    std::vector<unsigned char> reached(vertex_count);
    std::vector<int> distinct;
    for (int vertex : terminals)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || vertex >= n)
        {
            throw std::out_of_range("minimum_steiner_tree: terminal out of range");
        }
#endif
        if (!reached[vertex])
        {
            reached[vertex] = 1;
            distinct.push_back(vertex);
        }
    }
    if (distinct.size() == 1)
    {
        return SteinerTree{0, {}};
    }

    using Mask = std::size_t;
    using Cost = std::uint64_t;
    constexpr Cost kUnreachable = Cost{1} << std::numeric_limits<std::int64_t>::digits;
    const auto variable_count = distinct.size() - 1;
    if (variable_count >= std::numeric_limits<Mask>::digits)
    {
        throw std::length_error("minimum_steiner_tree: too many terminals for a subset mask");
    }
    const Mask subset_count = Mask{1} << variable_count;
    struct Choice
    {
        std::size_t value = 0;
        bool via_edge = false;
    };
    std::vector<Cost> dp;
    std::vector<Choice> trace;
    if (subset_count > std::min(dp.max_size(), trace.max_size()) / vertex_count)
    {
        throw std::length_error("minimum_steiner_tree: subset tables are too large");
    }

    struct Arc
    {
        int to;
        std::size_t id;
    };
    std::vector<std::vector<Arc>> graph(vertex_count);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &[from, to, weight] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("minimum_steiner_tree: endpoint out of range");
        }
        if (weight < 0)
        {
            throw std::invalid_argument("minimum_steiner_tree: negative edge weight");
        }
#endif
        graph[from].push_back({to, id});
        graph[to].push_back({from, id});
    }

    const auto state_count = subset_count * vertex_count;
    dp.assign(state_count, kUnreachable);
    trace.resize(state_count);
    using QueueEntry = std::pair<Cost, int>;
    std::vector<QueueEntry> queue;
    constexpr std::greater<QueueEntry> kGreater{};
    for (Mask mask = 1; mask < subset_count; ++mask)
    {
        const auto base = mask * vertex_count;
        if (std::has_single_bit(mask))
        {
            dp[base + distinct[std::countr_zero(mask) + 1]] = 0;
        }
        else
        {
            // 子集合対の片側に最下位ビットを固定し、各分割を一度だけ試す。
            const Mask pivot = Mask{1} << std::countr_zero(mask);
            const Mask rest = mask ^ pivot;
            for (Mask part = (rest - 1) & rest;; part = (part - 1) & rest)
            {
                const Mask left = part | pivot;
                const auto left_base = left * vertex_count;
                const auto right_base = (mask ^ left) * vertex_count;
                for (std::size_t v = 0; v < vertex_count; ++v)
                {
                    const auto first = dp[left_base + v];
                    const auto second = dp[right_base + v];
                    if (first != kUnreachable && second != kUnreachable)
                    {
                        // 有限値は最大INT64_MAX。2つ足してもuint64_tに収まる。
                        const auto merged = first + second;
                        if (merged < dp[base + v])
                        {
                            dp[base + v] = merged;
                            trace[base + v] = {left, false};
                        }
                    }
                }
                if (part == 0)
                {
                    break;
                }
            }
        }

        queue.clear();
        for (int v = 0; v < n; ++v)
        {
            if (dp[base + v] != kUnreachable)
            {
                queue.emplace_back(dp[base + v], v);
            }
        }
        std::make_heap(queue.begin(), queue.end(), kGreater);
        while (!queue.empty())
        {
            std::pop_heap(queue.begin(), queue.end(), kGreater);
            const auto [cost, vertex] = queue.back();
            queue.pop_back();
            if (cost != dp[base + vertex])
            {
                continue;
            }
            for (const auto &[to, id] : graph[vertex])
            {
                const auto next_cost = cost + static_cast<Cost>(edges[id].weight);
                if (next_cost < dp[base + to])
                {
                    dp[base + to] = next_cost;
                    trace[base + to] = {id, true};
                    queue.emplace_back(next_cost, to);
                    std::push_heap(queue.begin(), queue.end(), kGreater);
                }
            }
        }
    }

    const Mask all = subset_count - 1;
    const int root = distinct.front();
    const auto best = dp[all * vertex_count + root];
    const bool representable = best != kUnreachable;
    std::vector<unsigned char> selected(edges.size(), static_cast<unsigned char>(!representable));
    if (representable)
    {
        std::vector<std::pair<Mask, int>> pending{{all, root}};
        while (!pending.empty())
        {
            const auto [mask, vertex] = pending.back();
            pending.pop_back();
            const auto choice = std::exchange(trace[mask * vertex_count + vertex], Choice{});
            if (choice.via_edge)
            {
                selected[choice.value] = 1;
                const auto &edge = edges[choice.value];
                const int from = edge.to == vertex ? edge.from : edge.to;
                pending.emplace_back(mask, from);
            }
            else if (choice.value != 0)
            {
                pending.emplace_back(choice.value, vertex);
                pending.emplace_back(mask ^ choice.value, vertex);
            }
        }
    }

    // 復元した辺の和集合から木を取り出す。有限解がなければ元グラフで到達性を調べる。
    std::fill(reached.begin(), reached.end(), 0);
    reached[root] = 1;
    std::vector<int> order{root};
    SteinerTree result{representable ? static_cast<std::int64_t>(best) : 0, {}};
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        for (const auto &[to, id] : graph[order[i]])
        {
            if (selected[id] && !reached[to])
            {
                reached[to] = 1;
                order.push_back(to);
                if (representable)
                {
                    result.edge_ids.push_back(id);
                }
            }
        }
    }
    if (!representable)
    {
        if (std::ranges::all_of(distinct, [&](int terminal)
                                {
                                    return reached[terminal] != 0;
                                }))
        {
            throw std::overflow_error("minimum_steiner_tree: minimum cost does not fit int64_t");
        }
        return std::nullopt;
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
