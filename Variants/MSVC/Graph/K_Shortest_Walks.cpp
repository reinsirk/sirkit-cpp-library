// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Graph/K_Shortest_Walks.cpp
// 参考（最短路木から外れる辺による列挙）: https://doi.org/10.1137/S0097539795290477
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/k_shortest_walks.md
//
// API一覧
// KShortestWalkEdge: 有向辺{from,to,weight}。頂点番号は0始まり。重みは非負。
// k_shortest_walk_lengths(n,source,target,edges,k): 長さの小さい順に最大k個を返す。
// 自己辺・多重辺・辺の再利用を許す。同じ重さでも辺列が異なるウォークは個別に数える。
// source==targetなら空ウォークも数える。存在数がk未満ならその分だけ返す。
// k番目までに符号付き64ビットの範囲を超える長さがあればoverflow_error。
// O(n+m log(m+1)+k log(k+1))時間・O(n+m log(m+1)+k)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <initializer_list>
#include <limits>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_K_SHORTEST_WALKS_DEFINED
#define SIRKIT_K_SHORTEST_WALKS_DEFINED

struct KShortestWalkEdge
{
    int from;
    int to;
    std::int64_t weight;
};

namespace detail
{
inline constexpr auto kWalkNoNode = std::numeric_limits<std::size_t>::max();
inline constexpr auto kWalkUnreachable = std::numeric_limits<std::uint64_t>::max();
inline constexpr auto kWalkOverflow = std::uint64_t{1} << 63;

inline std::uint64_t add_walk_lengths(std::uint64_t a, std::uint64_t b)
{
    return a <= kWalkOverflow - b ? a + b : kWalkOverflow;
}

using WalkDistanceQueue = std::priority_queue<
    std::pair<std::uint64_t, std::size_t>,
    std::vector<std::pair<std::uint64_t, std::size_t>>,
    std::greater<>>;

struct WalkShortestTree
{
    std::vector<std::uint64_t> distance;
    std::vector<std::size_t> next_edge;
    std::vector<int> settled;
};

inline WalkShortestTree shortest_tree_to_walk_target(
    int n, int target, std::span<const KShortestWalkEdge> edges)
{
    const auto count = static_cast<std::size_t>(n);
    std::vector<std::size_t> offsets(count + 1);
    for (const auto &edge : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (edge.from < 0 || edge.from >= n || edge.to < 0 || edge.to >= n)
        {
            throw std::out_of_range("k_shortest_walk_lengths: edge endpoint out of range");
        }
        if (edge.weight < 0)
        {
            throw std::invalid_argument("k_shortest_walk_lengths: negative edge weight");
        }
#endif
        ++offsets[static_cast<std::size_t>(edge.to) + 1];
    }
    for (std::size_t i = 1; i < offsets.size(); ++i)
    {
        offsets[i] += offsets[i - 1];
    }

    std::vector<std::size_t> incoming(edges.size());
    {
        auto cursor = offsets;
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            incoming[cursor[edges[i].to]++] = i;
        }
    }

    WalkShortestTree tree{
        std::vector<std::uint64_t>(count, kWalkUnreachable),
        std::vector<std::size_t>(count, kWalkNoNode),
        {},
    };
    WalkDistanceQueue queue;
    tree.distance[target] = 0;
    queue.emplace(0, target);
    while (!queue.empty())
    {
        const auto [distance, v] = queue.top();
        queue.pop();
        if (distance != tree.distance[v])
        {
            continue;
        }
        tree.settled.push_back(static_cast<int>(v));
        for (auto p = offsets[v]; p < offsets[v + 1]; ++p)
        {
            const auto index = incoming[p];
            const auto &edge = edges[index];
            const auto next = add_walk_lengths(distance, static_cast<std::uint64_t>(edge.weight));
            if (next < tree.distance[edge.from])
            {
                tree.distance[edge.from] = next;
                // 確定済み頂点への辺を選ぶので、零重みの閉路でも木は循環しない。
                tree.next_edge[edge.from] = index;
                queue.emplace(next, edge.from);
            }
        }
    }
    return tree;
}

class WalkSidetrackHeap
{
public:
    struct Node
    {
        std::size_t left = kWalkNoNode;
        std::size_t right = kWalkNoNode;
        std::uint64_t extra = 0;
        int to = 0;
        int rank = 1;
    };

    const Node &operator[](std::size_t index) const
    {
        return nodes_[index];
    }

    std::size_t insert(std::size_t root, std::uint64_t extra, int to)
    {
        nodes_.push_back(Node{.extra = extra, .to = to});
        return meld<false>(root, nodes_.size() - 1);
    }

    std::size_t inherit(std::size_t local, std::size_t ancestor)
    {
        return ancestor == kWalkNoNode ? local : meld<true>(local, ancestor);
    }

private:
    std::deque<Node> nodes_;

    int rank(std::size_t node) const
    {
        return node == kWalkNoNode ? 0 : nodes_[node].rank;
    }

    // bは空でない。コピーは祖先のヒープを受け継ぐときだけ行う。
    template <bool Persistent>
    std::size_t meld(std::size_t a, std::size_t b)
    {
        if (a == kWalkNoNode)
        {
            return b;
        }
        if (nodes_[b].extra < nodes_[a].extra)
        {
            std::swap(a, b);
        }
        // 既存の版にあるノードは書き換えず、結合に通った箇所だけ複製する。
        auto node = nodes_[a];
        node.right = meld<Persistent>(node.right, b);
        if (rank(node.left) < rank(node.right))
        {
            std::swap(node.left, node.right);
        }
        node.rank = rank(node.right) + 1;
        if constexpr (Persistent)
        {
            nodes_.push_back(node);
            return nodes_.size() - 1;
        }
        else
        {
            nodes_[a] = node;
            return a;
        }
    }
};

inline std::int64_t checked_walk_length(std::uint64_t length)
{
    if (length == kWalkOverflow)
    {
        throw std::overflow_error("k_shortest_walk_lengths: walk length does not fit int64_t");
    }
    return static_cast<std::int64_t>(length);
}
} // namespace detail

inline std::vector<std::int64_t> k_shortest_walk_lengths(
    int n, int source, int target, std::span<const KShortestWalkEdge> edges, std::size_t k)
{
    if (n <= 0)
    {
        throw std::invalid_argument("k_shortest_walk_lengths: nonpositive vertex count");
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (source < 0 || source >= n || target < 0 || target >= n)
    {
        throw std::out_of_range("k_shortest_walk_lengths: source or target out of range");
    }
#endif
    if (k == 0)
    {
        return {};
    }
    const auto tree = detail::shortest_tree_to_walk_target(n, target, edges);
    const auto shortest = tree.distance[source];
    if (shortest == detail::kWalkUnreachable)
    {
        return {};
    }
    std::vector<std::int64_t> result{detail::checked_walk_length(shortest)};
    if (k == 1)
    {
        return result;
    }

    constexpr auto kNone = detail::kWalkNoNode;
    detail::WalkSidetrackHeap heap;
    std::vector<std::size_t> alternatives(n, kNone);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &edge = edges[i];
        if (tree.distance[edge.to] == detail::kWalkUnreachable || tree.next_edge[edge.from] == i)
        {
            continue;
        }
        // 64ビットに収まらない長さは上限直後へまとめる。そこまで到達した場合だけ例外にする。
        const auto length = detail::add_walk_lengths(static_cast<std::uint64_t>(edge.weight), tree.distance[edge.to]);
        const auto extra = length - tree.distance[edge.from];
        alternatives[edge.from] = heap.insert(alternatives[edge.from], extra, edge.to);
    }
    for (int v : tree.settled)
    {
        const auto parent = tree.next_edge[v];
        if (parent != kNone)
        {
            alternatives[v] = heap.inherit(alternatives[v], alternatives[edges[parent].to]);
        }
    }

    detail::WalkDistanceQueue queue;
    if (const auto first = alternatives[source]; first != kNone)
    {
        queue.emplace(detail::add_walk_lengths(shortest, heap[first].extra), first);
    }
    while (!queue.empty())
    {
        const auto [length, selected] = queue.top();
        queue.pop();
        result.push_back(detail::checked_walk_length(length));
        if (result.size() == k)
        {
            break;
        }
        const auto &node = heap[selected];
        // 最後の木外辺を置き換えるか、着点から新たな木外辺を加える。
        for (const auto child : {node.left, node.right})
        {
            if (child != kNone)
            {
                queue.emplace(detail::add_walk_lengths(length, heap[child].extra - node.extra), child);
            }
        }
        if (const auto next = alternatives[node.to]; next != kNone)
        {
            queue.emplace(detail::add_walk_lengths(length, heap[next].extra), next);
        }
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
