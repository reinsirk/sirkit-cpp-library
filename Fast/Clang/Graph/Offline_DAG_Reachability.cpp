// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Offline_DAG_Reachability.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/offline_dag_reachability.md
//
// API一覧
// offline_dag_reachability(n,edges,queries): DAGで始点から終点に到達できるか、入力順のvector<bool>で返す。
// edgesとqueriesは(始点,終点)の列。頂点番号は[0,n)。平行辺を許す。同じ頂点への照会はtrue。
// トポロジカル順に逆らう照会はfalse。残りの始点数・終点数の小さい方をpとする。
// O((n+m)(1+ceil(p/64))+q)時間・O(n+m+q)領域。mは辺数、qは照会数。
// 通常版で有向閉路を含む入力はinvalid_argument。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_OFFLINE_DAG_REACHABILITY_DEFINED
#define SIRKIT_OFFLINE_DAG_REACHABILITY_DEFINED

namespace detail
{
inline std::vector<int> reachability_topological_ranks(int n, std::span<const std::pair<int, int>> edges,
                                                        bool already_sorted)
{
    std::vector<int> ranks(n);
    if (already_sorted)
    {
        std::iota(ranks.begin(), ranks.end(), 0);
        return ranks;
    }
    std::vector<std::vector<int>> outgoing(n);
    std::vector<std::size_t> inbound(n);
    for (const auto &[u, v] : edges)
    {
        outgoing[u].push_back(v);
        ++inbound[v];
    }
    std::vector<int> order;
    order.reserve(n);
    for (int v = 0; v < n; ++v)
    {
        if (inbound[v] == 0)
        {
            order.push_back(v);
        }
    }
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        const int v = order[i];
        ranks[v] = static_cast<int>(i);
        for (const int u : outgoing[v])
        {
            if (--inbound[u] == 0)
            {
                order.push_back(u);
            }
        }
    }
    return ranks;
}
}

inline std::vector<bool> offline_dag_reachability(int n, std::span<const std::pair<int, int>> edges,
                                                    std::span<const std::pair<int, int>> queries)
{
    if (n < 0)
    {
        throw std::length_error("offline_dag_reachability: negative vertex count");
    }
    bool already_sorted = true;
    for (const auto &[u, v] : edges)
    {
        already_sorted &= u < v;
    }
    const auto ranks = detail::reachability_topological_ranks(n, edges, already_sorted);
    struct Query
    {
        int from;
        int to;
        std::size_t answer_index;
    };
    std::vector<bool> answer(queries.size());
    std::vector<Query> pending;
    pending.reserve(queries.size());
    std::vector<std::uint8_t> sources(n), targets(n);
    int unique_sources = 0, unique_targets = 0;
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        const auto [u, v] = queries[i];
        const int a = ranks[u], b = ranks[v];
        if (a == b)
        {
            answer[i] = true;
        }
        else if (a < b)
        {
            pending.push_back({a, b, i});
            unique_sources += sources[a] == 0;
            unique_targets += targets[b] == 0;
            sources[a] = targets[b] = 1;
        }
    }
    if (pending.empty())
    {
        return answer;
    }

    // グラフと照会を逆向きに読み替えれば、少ない側の端点だけをビットに割り当てられる。
    const bool reversed = unique_targets < unique_sources;
    const auto orient = [reversed, n](int from, int to)
    {
        return reversed ? std::pair{n - 1 - to, n - 1 - from} : std::pair{from, to};
    };
    std::vector<std::size_t> edge_begin(static_cast<std::size_t>(n) + 1), query_begin(edge_begin.size());
    for (const auto &[u, v] : edges)
    {
        const auto [from, to] = orient(ranks[u], ranks[v]);
        ++edge_begin[from + 1];
    }
    for (auto &[from, to, index] : pending)
    {
        const auto oriented = orient(from, to);
        from = oriented.first;
        to = oriented.second;
        ++query_begin[from + 1];
    }
    std::partial_sum(edge_begin.begin(), edge_begin.end(), edge_begin.begin());
    std::partial_sum(query_begin.begin(), query_begin.end(), query_begin.begin());
    auto edge_cursor = edge_begin, query_cursor = query_begin;
    std::vector<int> destinations(edges.size());
    for (const auto &[u, v] : edges)
    {
        const auto [from, to] = orient(ranks[u], ranks[v]);
        destinations[edge_cursor[from]++] = to;
    }
    std::vector<Query> grouped(pending.size());
    for (const auto &query : pending)
    {
        grouped[query_cursor[query.from]++] = query;
    }

    std::vector<std::uint64_t> reach(n);
    for (int first = 0; first < n; )
    {
        std::array<int, 64> seeds;
        std::size_t size = 0;
        for (; first < n && size < seeds.size(); ++first)
        {
            if (query_begin[first] != query_begin[first + 1])
            {
                seeds[size++] = first;
            }
        }
        if (size == 0)
        {
            break;
        }
        int last_target = seeds[0];
        std::fill(reach.begin(), reach.end(), 0);
        for (std::size_t bit = 0; bit < size; ++bit)
        {
            const int start = seeds[bit];
            reach[start] = std::uint64_t{1} << bit;
            for (auto i = query_begin[start]; i != query_begin[start + 1]; ++i)
            {
                last_target = std::max(last_target, grouped[i].to);
            }
        }
        for (int v = seeds[0]; v < last_target; ++v)
        {
            if (const auto bits = reach[v]; bits != 0)
            {
                for (auto i = edge_begin[v]; i != edge_begin[v + 1]; ++i)
                {
                    reach[destinations[i]] |= bits;
                }
            }
        }
        for (std::size_t bit = 0; bit < size; ++bit)
        {
            const int start = seeds[bit];
            for (auto i = query_begin[start]; i != query_begin[start + 1]; ++i)
            {
                const auto &query = grouped[i];
                answer[query.answer_index] = (reach[query.to] >> bit) & 1;
            }
        }
    }
    return answer;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
