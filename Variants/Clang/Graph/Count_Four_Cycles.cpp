// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Count_Four_Cycles.cpp
// 参考: Chiba & Nishizeki, Arboricity and Subgraph Listing Algorithms, https://doi.org/10.1137/0214017
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/count_four_cycles.md
//
// API一覧
// count_four_cycles_per_edge(n,edges): 各入力辺を含む長さ4の単純閉路の個数を入力順に返す。
// 頂点は0..n-1。無向多重辺は別々に数え、自己辺の答えは0。閉路の始点や向きを数え分けない。
// 入力辺数m≤4,000,000、自己辺と並行辺を除いた辺数dとしてO(n+m log(m+1)+d√d)時間・O(n+m)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COUNT_FOUR_CYCLES_DEFINED
#define SIRKIT_COUNT_FOUR_CYCLES_DEFINED
namespace detail
{
inline void CheckFourCycleEdgeCount(std::size_t count)
{
    constexpr std::size_t kMaxEdges = 4'000'000;
    if (count > kMaxEdges)
    {
        // 固定した1辺に対し、残り3辺の組合せの上界をuint64_tに収める。
        throw std::length_error("count_four_cycles_per_edge: too many edges");
    }
}
}

inline std::vector<std::uint64_t> count_four_cycles_per_edge(int n, std::span<const std::pair<int, int>> edges)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 0)
    {
        throw std::invalid_argument("count_four_cycles_per_edge: negative vertex count");
    }
#endif
    detail::CheckFourCycleEdgeCount(edges.size());
    using Endpoints = std::pair<int, int>;
    std::vector<std::pair<Endpoints, std::size_t>> sorted;
    sorted.reserve(edges.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        auto [u, v] = edges[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("count_four_cycles_per_edge: invalid endpoint");
        }
#endif
        if (u != v)
        {
            sorted.emplace_back(std::minmax(u, v), i);
        }
    }
    std::sort(sorted.begin(), sorted.end());
    struct Group
    {
        Endpoints endpoints;
        std::uint64_t multiplicity = 0;
        std::uint64_t answer = 0;
    };
    std::vector<Group> groups;
    std::vector<std::size_t> assignment(edges.size(), edges.size());
    std::vector<std::size_t> offset(static_cast<std::size_t>(n) + 1);
    for (const auto &[endpoints, id] : sorted)
    {
        if (groups.empty() || groups.back().endpoints != endpoints)
        {
            groups.push_back({endpoints});
            ++offset[endpoints.first + 1];
            ++offset[endpoints.second + 1];
        }
        ++groups.back().multiplicity;
        assignment[id] = groups.size() - 1;
    }
    std::partial_sum(offset.begin(), offset.end(), offset.begin());
    struct Arc
    {
        int to;
        std::size_t group;
    };
    std::vector<Arc> arcs(offset.back());
    auto next = offset;
    for (std::size_t id = 0; id < groups.size(); ++id)
    {
        const auto [u, v] = groups[id].endpoints;
        arcs[next[u]++] = {v, id};
        arcs[next[v]++] = {u, id};
    }
    const auto vertex_key = [&](int v)
    {
        return std::pair{offset[v + 1] - offset[v], v};
    };
    std::vector<std::uint64_t> paths(n);
    std::vector<int> touched;
    for (int u = 0; u < n; ++u)
    {
        const auto key = vertex_key(u);
        // 各四角形は(次数, 頂点番号)が最大の頂点uで処理する。
        const auto visit_wedges = [&](auto action)
        {
            for (auto i = offset[u]; i < offset[u + 1]; ++i)
            {
                const auto [middle, first] = arcs[i];
                if (vertex_key(middle) >= key)
                {
                    continue;
                }
                for (auto j = offset[middle]; j < offset[middle + 1]; ++j)
                {
                    const auto [opposite, second] = arcs[j];
                    if (vertex_key(opposite) < key)
                    {
                        action(opposite, groups[first], groups[second]);
                    }
                }
            }
        };
        visit_wedges([&](int v, const Group &first, const Group &second)
                     {
                         if (paths[v] == 0)
                         {
                             touched.push_back(v);
                         }
                         paths[v] += first.multiplicity * second.multiplicity;
                     });
        visit_wedges([&](int v, Group &first, Group &second)
                     {
                         const auto alternatives = paths[v] - first.multiplicity * second.multiplicity;
                         first.answer += second.multiplicity * alternatives;
                         second.answer += first.multiplicity * alternatives;
                     });
        for (const int v : touched)
        {
            paths[v] = 0;
        }
        touched.clear();
    }
    std::vector<std::uint64_t> answer(edges.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        if (assignment[i] != edges.size())
        {
            answer[i] = groups[assignment[i]].answer;
        }
    }
    return answer;
}
#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
