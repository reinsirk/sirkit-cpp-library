// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Enumerate_Cliques.cpp
// 参考（線形時間のコア分解）: https://arxiv.org/abs/cs/0310049
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/enumerate_cliques.md
//
// API一覧
// enumerate_cliques(n,edges,visit): 単純無向グラフの非空クリークごとにvisit(span<const int>)を1回呼ぶ。
// 頂点番号は[0,n)、自己辺・重複辺は不可。孤立頂点も1頂点のクリークとして列挙する。
// 列挙順と頂点順は任意。spanは呼び出し中だけ有効。
// 退化度d、クリーク数Cに対し、コールバックを除きO(n+m+(d+1)C)時間・O(n+m)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ENUMERATE_CLIQUES_DEFINED
#define SIRKIT_ENUMERATE_CLIQUES_DEFINED

namespace detail
{
struct CliqueOrder
{
    std::vector<int> vertices;
    std::vector<std::vector<int>> forward;
    std::size_t max_forward = 0;
};

inline CliqueOrder make_clique_order(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("enumerate_cliques: negative vertex count");
    }
    const auto count = static_cast<std::size_t>(n);
    CliqueOrder result{std::vector<int>(count), std::vector<std::vector<int>>(count)};
    auto &order = result.vertices;
    std::vector<int> position(count);
    std::vector<std::vector<int>> adjacency(count);
    for (const auto &[from, to] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("enumerate_cliques: endpoint out of range");
        }
        if (from == to)
        {
            throw std::invalid_argument("enumerate_cliques: self loop");
        }
#endif
        adjacency[from].push_back(to);
        adjacency[to].push_back(from);
    }

    std::vector<std::size_t> degree(count), bucket(count + 1);
    for (int v = 0; v < n; ++v)
    {
        const auto size = adjacency[v].size();
#ifndef SKIP_RUNTIME_CHECKS
        if (size >= count)
        {
            throw std::invalid_argument("enumerate_cliques: degree exceeds simple graph bound");
        }
#endif
        degree[v] = size;
        ++bucket[size];
    }
    std::size_t start = 0;
    for (auto &slot : bucket)
    {
        const auto frequency = slot;
        slot = start;
        start += frequency;
    }
    for (int v = 0; v < n; ++v)
    {
        const auto rank = bucket[degree[v]]++;
        position[v] = static_cast<int>(rank);
        order[rank] = v;
    }
    for (std::size_t d = count; d != 0; --d)
    {
        bucket[d] = bucket[d - 1];
    }
    bucket[0] = 0;

#ifndef SKIP_RUNTIME_CHECKS
    std::vector<int> last_neighbor_of(count, -1);
#endif
    // 次数の大きい隣接頂点を、一つ前のバケットへ移す。
    for (int v : order)
    {
        for (int to : adjacency[v])
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (last_neighbor_of[to] == v)
            {
                throw std::invalid_argument("enumerate_cliques: duplicate edge");
            }
            last_neighbor_of[to] = v;
#endif
            const auto d = degree[to];
            if (d > degree[v])
            {
                const auto first = bucket[d]++;
                const int other = order[first];
                std::swap(order[position[to]], order[first]);
                std::swap(position[to], position[other]);
                --degree[to];
            }
        }
    }

    // 行き先の順位が昇順になるよう、後ろ側の頂点から辺を追加する。
    for (int rank = 0; rank < n; ++rank)
    {
        for (int from : adjacency[order[rank]])
        {
            const int earlier = position[from];
            if (earlier < rank)
            {
                result.forward[earlier].push_back(rank);
                result.max_forward = std::max(result.max_forward, result.forward[earlier].size());
            }
        }
    }
    return result;
}
} // namespace detail

template <class Visit>
void enumerate_cliques(int n, std::span<const std::pair<int, int>> edges, Visit &&visit)
{
    const auto [order, forward, max_forward] = detail::make_clique_order(n, edges);
    std::vector<std::vector<int>> buffers(max_forward + 1);
    std::vector<int> clique;
    clique.reserve(max_forward + 1);
    const auto search = [&](auto &&self, int rank, std::span<const int> candidates) -> void
    {
        clique.push_back(order[rank]);
        visit(std::span<const int>(clique));
        auto &next = buffers[clique.size() - 1];
        for (std::size_t i = 0; i < candidates.size(); ++i)
        {
            const int to = candidates[i];
            next.clear();
            std::ranges::set_intersection(candidates.subspan(i + 1), forward[to], std::back_inserter(next));
            self(self, to, next);
        }
        clique.pop_back();
    };
    for (int rank = 0; rank < n; ++rank)
    {
        search(search, rank, forward[rank]);
    }
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
