// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Hamiltonian_Route.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/hamiltonian_route.md
// 参考（Held–Karp の部分集合DP）: https://doi.org/10.1137/0110015
//
// API一覧
// shortest_hamiltonian_path<Weight=int64_t>(n,start,cost_at,end=nullopt): startから全頂点をちょうど1度通る最小コストのパス。
// shortest_hamiltonian_cycle<Weight=int64_t>(n,start,cost_at): startから全頂点をちょうど1度通り、startへ戻る最小コストの巡回。
// 共通: {cost,vertices}。解がなければnullopt。cost_at(u,v)は辺コストかoptional。空なら辺なし。
// 負辺・有向辺を許す。頂点番号は0始まり。途中の和もWeightに収まること。O(n^2 2^n)時間・O(n 2^n)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_HAMILTONIAN_ROUTE_DEFINED
#define SIRKIT_HAMILTONIAN_ROUTE_DEFINED

template <class Weight>
struct HamiltonianRoute
{
    Weight cost{};
    std::vector<std::size_t> vertices;
};

namespace detail
{
inline std::size_t HamiltonianCell(std::size_t mask, std::size_t last, std::size_t stride)
{
    // 終点lastは必ずmask内にあるので、配列ではそのビットを省く。
    const auto low = (std::size_t{1} << last) - 1;
    return last * stride + ((mask & low) | ((mask >> 1) & ~low));
}

template <bool ReturnHome, class Weight, class CostAt>
std::optional<HamiltonianRoute<Weight>> FindHamiltonianRoute(std::size_t n, std::size_t start, const CostAt &cost_at,
                                                             std::optional<std::size_t> end)
{
    if (start >= n || (end && *end >= n))
    {
        throw std::out_of_range("shortest_hamiltonian_route: invalid vertex");
    }
    if (n == 1)
    {
        if constexpr (ReturnHome)
        {
            return HamiltonianRoute<Weight>{Weight{}, {start, start}};
        }
        else
        {
            return HamiltonianRoute<Weight>{Weight{}, {start}};
        }
    }
    if (end && *end == start)
    {
        return std::nullopt;
    }

    const auto tail = n - 1;
    if (tail >= std::numeric_limits<std::size_t>::digits)
    {
        throw std::length_error("shortest_hamiltonian_route: subset mask overflows");
    }
    const auto subsets = std::size_t{1} << tail;
    const auto stride = subsets / 2;
    std::vector<std::optional<Weight>> dp;
    if (stride > dp.max_size() / tail)
    {
        throw std::length_error("shortest_hamiltonian_route: DP table is too large");
    }
    dp.resize(tail * stride);
    std::vector<std::uint8_t> predecessor(dp.size());

    std::vector<std::size_t> vertices;
    vertices.reserve(tail);
    for (std::size_t vertex = 0; vertex < n; ++vertex)
    {
        if (vertex != start)
        {
            vertices.push_back(vertex);
        }
    }
    std::vector<std::optional<Weight>> edges(n * n);
    for (std::size_t u = 0; u < n; ++u)
    {
        for (std::size_t v = 0; v < n; ++v)
        {
            if (u != v)
            {
                edges[u * n + v] = std::optional<Weight>{cost_at(u, v)};
            }
        }
    }
    for (std::size_t mask = 1; mask < subsets; ++mask)
    {
        for (auto ends = mask; ends != 0; ends &= ends - 1)
        {
            const auto last = static_cast<std::size_t>(std::countr_zero(ends));
            const auto index = HamiltonianCell(mask, last, stride);
            const auto v = vertices[last];
            auto &best = dp[index];
            const auto prefix = mask ^ (std::size_t{1} << last);
            if (prefix == 0)
            {
                best = edges[start * n + v];
                continue;
            }
            for (auto prior = prefix; prior != 0; prior &= prior - 1)
            {
                const auto previous = static_cast<std::size_t>(std::countr_zero(prior));
                const auto &before = dp[HamiltonianCell(prefix, previous, stride)];
                const auto &edge = edges[vertices[previous] * n + v];
                if (before && edge)
                {
                    auto candidate = *before;
                    candidate += *edge;
                    if (!best || candidate < *best)
                    {
                        best = std::move(candidate);
                        predecessor[index] = static_cast<std::uint8_t>(previous);
                    }
                }
            }
        }
    }
    auto mask = subsets - 1;
    std::optional<Weight> best;
    std::size_t best_last = 0;
    for (std::size_t last = 0; last < tail; ++last)
    {
        if (end && *end != vertices[last])
        {
            continue;
        }
        auto score = dp[HamiltonianCell(mask, last, stride)];
        if (!score)
        {
            continue;
        }
        if constexpr (ReturnHome)
        {
            const auto &closing_edge = edges[vertices[last] * n + start];
            if (!closing_edge)
            {
                continue;
            }
            *score += *closing_edge;
        }
        if (!best || *score < *best)
        {
            best = std::move(score);
            best_last = last;
        }
    }
    if (!best)
    {
        return std::nullopt;
    }
    std::vector<std::size_t> route;
    route.reserve(n + ReturnHome);
    while (mask != 0)
    {
        route.push_back(vertices[best_last]);
        const auto index = HamiltonianCell(mask, best_last, stride);
        mask ^= std::size_t{1} << best_last;
        best_last = predecessor[index];
    }
    route.push_back(start);
    std::ranges::reverse(route);
    if constexpr (ReturnHome)
    {
        route.push_back(start);
    }
    return HamiltonianRoute<Weight>{*best, std::move(route)};
}
} // namespace detail

template <class Weight = std::int64_t, class CostAt>
std::optional<HamiltonianRoute<Weight>> shortest_hamiltonian_path(std::size_t n, std::size_t start, const CostAt &cost_at,
                                                                  std::optional<std::size_t> end = std::nullopt)
{
    return detail::FindHamiltonianRoute<false, Weight>(n, start, cost_at, end);
}

template <class Weight = std::int64_t, class CostAt>
std::optional<HamiltonianRoute<Weight>> shortest_hamiltonian_cycle(std::size_t n, std::size_t start, const CostAt &cost_at)
{
    return detail::FindHamiltonianRoute<true, Weight>(n, start, cost_at, std::nullopt);
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
