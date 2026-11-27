// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Minimum_Directed_Spanning_Tree.cpp
// 参考（有向閉路の縮約と優先度付きキュー）: https://doi.org/10.1002/net.3230070103
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/minimum_directed_spanning_tree.md
//
// API一覧
// DirectedMstEdge: 有向辺{from,to,weight}。頂点番号は0始まり。負の重み・自己辺・多重辺に対応。
// minimum_directed_spanning_tree(n,root,edges): rootから全頂点へ到達する最小全域有向木を返す。存在しなければstd::nullopt。
// DirectedMstResult: costは重みの合計。incoming_edge[v]は頂点vに入る辺の入力添字。根の欄はkNoEdge。
// 辺・合計は符号付き64ビット。最小費用が範囲外ならoverflow_error。
// O(N+M log(M+1))時間（単純グラフならO(N+M log(N+1))）・O(N+M)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MINIMUM_DIRECTED_SPANNING_TREE_DEFINED
#define SIRKIT_MINIMUM_DIRECTED_SPANNING_TREE_DEFINED

struct DirectedMstEdge
{
    int from;
    int to;
    std::int64_t weight;
};

struct DirectedMstResult
{
    static constexpr std::size_t kNoEdge = std::numeric_limits<std::size_t>::max();
    std::int64_t cost;
    std::vector<std::size_t> incoming_edge;
};

namespace detail
{
inline constexpr std::size_t kDirectedMstNone = DirectedMstResult::kNoEdge;

class DirectedMstHeap
{
    struct Node
    {
        std::size_t left = kDirectedMstNone;
        std::size_t right = kDirectedMstNone;
        std::uint64_t key = 0;
        std::uint64_t decrease = 0;
        int rank = 1;
    };

    std::vector<Node> nodes_;

    int rank(std::size_t node) const
    {
        return node == kDirectedMstNone ? 0 : nodes_[node].rank;
    }

    void push(std::size_t node)
    {
        auto &entry = nodes_[node];
        reduce(entry.left, entry.decrease);
        reduce(entry.right, entry.decrease);
        entry.decrease = 0;
    }

public:
    explicit DirectedMstHeap(std::size_t size) : nodes_(size)
    {
    }

    void initialize(std::size_t edge, std::int64_t weight)
    {
        // 全ての解の辺数が等しいため、重みに同じ定数を足してよい。
        constexpr auto kSign = std::uint64_t{1} << 63;
        nodes_[edge].key = static_cast<std::uint64_t>(weight) ^ kSign;
    }

    std::uint64_t key(std::size_t node) const
    {
        return nodes_[node].key;
    }

    void reduce(std::size_t node, std::uint64_t amount)
    {
        if (node != kDirectedMstNone)
        {
            // 縮約による各辺の減少量は、元の非負のキー以下に収まる。
            nodes_[node].key -= amount;
            nodes_[node].decrease += amount;
        }
    }

    std::size_t meld(std::size_t a, std::size_t b)
    {
        if (a == kDirectedMstNone)
        {
            return b;
        }
        if (b == kDirectedMstNone)
        {
            return a;
        }
        if (nodes_[a].key > nodes_[b].key)
        {
            std::swap(a, b);
        }
        push(a);
        auto &entry = nodes_[a];
        entry.right = meld(entry.right, b);
        if (rank(entry.left) < rank(entry.right))
        {
            std::swap(entry.left, entry.right);
        }
        entry.rank = rank(entry.right) + 1;
        return a;
    }

    std::size_t pop(std::size_t node)
    {
        push(node);
        return meld(nodes_[node].left, nodes_[node].right);
    }
};

class DirectedMstDisjointSet
{
    std::vector<int> parents_;

public:
    explicit DirectedMstDisjointSet(int n) : parents_(n, -1)
    {
    }

    int find(int vertex)
    {
        while (parents_[vertex] >= 0)
        {
            const int parent = parents_[vertex];
            if (parents_[parent] < 0)
            {
                return parent;
            }
            vertex = parents_[vertex] = parents_[parent];
        }
        return vertex;
    }

    int join_roots(int a, int b)
    {
        if (parents_[a] > parents_[b])
        {
            std::swap(a, b);
        }
        parents_[a] += parents_[b];
        parents_[b] = a;
        return a;
    }
};

struct DirectedMstComponent
{
    std::size_t heap = kDirectedMstNone;
    std::size_t incoming = kDirectedMstNone;
    std::size_t first_child = kDirectedMstNone;
    std::size_t sibling = kDirectedMstNone;
    int representative = 0;
    int seen_from = -1;
};

struct DirectedMstInterval
{
    int begin = -1;
    int end = -1;
};

inline std::vector<DirectedMstInterval> directed_mst_intervals(
    int n, const std::vector<DirectedMstComponent> &components)
{
    std::vector<DirectedMstInterval> intervals(components.size());
    std::vector<std::pair<std::size_t, bool>> pending;
    int leaves = 0;
    // 親の縮約番号は子より大きいので、各縮約木の根から順に始められる。
    for (std::size_t i = components.size(); i != 0; --i)
    {
        const auto top = i - 1;
        if (intervals[top].begin >= 0)
        {
            continue;
        }
        pending.emplace_back(top, false);
        while (!pending.empty())
        {
            const auto [node, finished] = pending.back();
            pending.pop_back();
            auto &interval = intervals[node];
            if (finished)
            {
                interval.end = leaves;
            }
            else if (node < static_cast<std::size_t>(n))
            {
                interval.begin = leaves;
                interval.end = ++leaves;
            }
            else
            {
                interval.begin = leaves;
                pending.emplace_back(node, true);
                for (auto child = components[node].first_child; child != kDirectedMstNone; child = components[child].sibling)
                {
                    pending.emplace_back(child, false);
                }
            }
        }
    }
    return intervals;
}

inline DirectedMstResult restore_directed_mst(
    int n, int root, std::span<const DirectedMstEdge> edges, std::vector<DirectedMstComponent> &components)
{
    if (components.size() > static_cast<std::size_t>(n))
    {
        const auto intervals = directed_mst_intervals(n, components);
        for (std::size_t i = components.size(); i > static_cast<std::size_t>(n); --i)
        {
            const auto &component = components[i - 1];
            const int target = intervals[edges[component.incoming].to].begin;
            // 外から入る辺の着点を含む子だけ、閉路内で選んだ入辺を置換する。
            for (auto child = component.first_child; child != kDirectedMstNone; child = components[child].sibling)
            {
                if (intervals[child].begin <= target && target < intervals[child].end)
                {
                    components[child].incoming = component.incoming;
                }
            }
        }
    }

    DirectedMstResult result{0, std::vector<std::size_t>(n, kDirectedMstNone)};
    std::uint64_t low = 0;
    std::int64_t high = 0;
    for (int vertex = 0; vertex < n; ++vertex)
    {
        if (vertex == root)
        {
            continue;
        }
        const auto index = components[vertex].incoming;
        result.incoming_edge[vertex] = index;
        const auto weight = edges[index].weight;
        const auto next = low + static_cast<std::uint64_t>(weight);
        // 128ビット相当で加算し、途中で64ビットを超えてから相殺する場合も扱う。
        high += static_cast<int>(next < low) - static_cast<int>(weight < 0);
        low = next;
    }
    result.cost = static_cast<std::int64_t>(low);
    if (high != -static_cast<int>(result.cost < 0))
    {
        throw std::overflow_error("minimum_directed_spanning_tree: total cost does not fit int64_t");
    }
    return result;
}
} // namespace detail

inline std::optional<DirectedMstResult> minimum_directed_spanning_tree(
    int n, int root, std::span<const DirectedMstEdge> edges)
{
    if (n <= 0)
    {
        throw std::invalid_argument("minimum_directed_spanning_tree: nonpositive vertex count");
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (root < 0 || root >= n)
    {
        throw std::out_of_range("minimum_directed_spanning_tree: root out of range");
    }
#endif
    constexpr auto kNone = detail::kDirectedMstNone;
    std::vector<detail::DirectedMstComponent> components(n);
    components.reserve(static_cast<std::size_t>(n) * 2 - 1);
    detail::DirectedMstHeap heap(edges.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &[from, to, weight] = edges[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("minimum_directed_spanning_tree: edge endpoint out of range");
        }
#endif
        if (to == root || from == to)
        {
            continue;
        }
        heap.initialize(i, weight);
        components[to].heap = heap.meld(components[to].heap, i);
    }

    detail::DirectedMstDisjointSet groups(n);
    std::vector<std::size_t> component_of(n);
    for (int vertex = 0; vertex < n; ++vertex)
    {
        components[vertex].representative = vertex;
        component_of[vertex] = static_cast<std::size_t>(vertex);
    }
    components[root].seen_from = root;
    std::vector<std::size_t> path;
    for (int start = 0; start < n; ++start)
    {
        auto current = component_of[groups.find(start)];
        while (components[current].seen_from < 0)
        {
            auto &component = components[current];
            component.seen_from = start;
            path.push_back(current);

            std::size_t source = kNone;
            while (component.heap != kNone)
            {
                const auto edge = component.heap;
                source = component_of[groups.find(edges[edge].from)];
                component.heap = heap.pop(edge);
                if (source != current)
                {
                    component.incoming = edge;
                    heap.reduce(component.heap, heap.key(edge));
                    break;
                }
            }
            if (component.incoming == kNone)
            {
                return std::nullopt;
            }
            if (components[source].seen_from != start)
            {
                current = source;
                continue;
            }

            // pathの末尾にある閉路を縮約し、調整済みの入辺ヒープを結合する。
            detail::DirectedMstComponent cycle;
            cycle.representative = component.representative;
            do
            {
                const auto child = path.back();
                path.pop_back();
                auto &member = components[child];
                member.sibling = cycle.first_child;
                cycle.first_child = child;
                cycle.heap = heap.meld(cycle.heap, member.heap);
                if (child != current)
                {
                    cycle.representative = groups.join_roots(cycle.representative, member.representative);
                }
            } while (cycle.first_child != source);
            current = components.size();
            component_of[cycle.representative] = current;
            components.push_back(cycle);
        }
        path.clear();
    }
    return detail::restore_directed_mst(n, root, edges, components);
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
