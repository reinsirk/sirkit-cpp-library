// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Bitwise_Path.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bitwise_path.md
//
// API一覧
// minimum_or_path(n,edges,source,target,directed=true): 辺ラベルのOR値が最小の単純パスを返す。
// maximum_and_path(n,edges,source,target,directed=true): 辺ラベルのAND値が最大の単純パスを返す。
//   BitwisePathEdge{from,to,label}。0始まり、ラベルはuint64_t。自己辺・多重辺も可。
//   BitwisePath{value,vertices,edge_ids}を返す。到達不能はnullopt。始点=終点は空パス、値は各演算の単位元。
//   Bを検査するビット数（最大64）とし、O((B+1)(n+m))時間、O(n+m)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BITWISE_PATH_DEFINED
#define SIRKIT_BITWISE_PATH_DEFINED

struct BitwisePathEdge
{
    int from;
    int to;
    std::uint64_t label;
};

struct BitwisePath
{
    std::uint64_t value;
    std::vector<int> vertices;
    std::vector<std::size_t> edge_ids;
};

namespace detail
{
[[nodiscard]] inline std::optional<BitwisePath> minimum_complemented_or_path(
    int n, std::span<const BitwisePathEdge> edges, int source, int target, bool directed, std::uint64_t invert)
{
    if (n < 0)
    {
        throw std::length_error("bitwise_path: negative vertex count");
    }
    struct Arc
    {
        int to;
        std::size_t id;
        std::uint64_t mask;
    };
    std::vector<std::vector<Arc>> adjacency(n);

    std::uint64_t possible = 0;
    auto common = std::numeric_limits<std::uint64_t>::max();
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &[u, v, label] = edges[id];
        const auto bits = label ^ invert;
        possible |= bits;
        common &= bits;
        adjacency[u].push_back({v, id, bits});
        if (!directed)
        {
            adjacency[v].push_back({u, id, bits});
        }
    }
    if (source == target)
    {
        return BitwisePath{invert, {source}, {}};
    }

    std::vector<int> previous(n), pending;
    std::vector<std::size_t> used_edge(n);
    auto can_reach = [&](std::uint64_t allowed)
    {
        std::ranges::fill(previous, -1);
        previous[source] = source;
        pending.clear();
        pending.push_back(source);
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            const int u = pending[i];
            for (const auto &[v, id, bits] : adjacency[u])
            {
                if (previous[v] == -1 && (bits & ~allowed) == 0)
                {
                    previous[v] = u;
                    used_edge[v] = id;
                    if (v == target)
                    {
                        return true;
                    }
                    pending.push_back(v);
                }
            }
        }
        return false;
    };

    if (!can_reach(possible))
    {
        return std::nullopt;
    }
    for (auto bits = possible & ~common; bits != 0;)
    {
        const auto highest = std::bit_floor(bits);
        const auto candidate = possible ^ highest;
        if (can_reach(candidate))
        {
            possible = candidate;
        }
        bits ^= highest;
    }
    can_reach(possible);

    BitwisePath result{possible ^ invert, {}, {}};
    for (int v = target; v != source; v = previous[v])
    {
        result.vertices.push_back(v);
        result.edge_ids.push_back(used_edge[v]);
    }
    result.vertices.push_back(source);
    std::ranges::reverse(result.vertices);
    std::ranges::reverse(result.edge_ids);
    return result;
}
} // namespace detail

[[nodiscard]] inline std::optional<BitwisePath> minimum_or_path(
    int n, std::span<const BitwisePathEdge> edges, int source, int target, bool directed = true)
{
    return detail::minimum_complemented_or_path(n, edges, source, target, directed, 0);
}

[[nodiscard]] inline std::optional<BitwisePath> maximum_and_path(
    int n, std::span<const BitwisePathEdge> edges, int source, int target, bool directed = true)
{
    return detail::minimum_complemented_or_path(n, edges, source, target, directed,
                                                std::numeric_limits<std::uint64_t>::max());
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
