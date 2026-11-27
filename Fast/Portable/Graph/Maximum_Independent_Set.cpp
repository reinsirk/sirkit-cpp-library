// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Maximum_Independent_Set.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/maximum_independent_set.md
//
// API一覧
// maximum_independent_set(n,edges): 最大個数の独立集合を、頂点番号の昇順で返す。
// maximum_weight_independent_set(weights,edges): 最大重みの独立集合を{weight,vertices}で返す。
// 重み版は非正の頂点を含めない。最適値がint64_tの上限を超えるとoverflow_error。
// どちらも頂点数は0以上46以下。重複辺は可、自己辺のある頂点は選ばない。
// 半分全列挙によりO(n+m+2^ceil(n/2))時間・O(n+2^floor(n/2))領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MAXIMUM_INDEPENDENT_SET_DEFINED
#define SIRKIT_MAXIMUM_INDEPENDENT_SET_DEFINED

inline constexpr int kMaxIndependentSetVertices = 46;

struct MaximumWeightIndependentSetResult
{
    std::int64_t weight;
    std::vector<int> vertices;
};

namespace detail
{
using IndependentSetMask = std::uint64_t;
inline constexpr auto kIndependentSetOverflow = std::uint64_t{1} << 63;

struct IndependentSetGraph
{
    std::array<IndependentSetMask, kMaxIndependentSetVertices> neighbors{};
    IndependentSetMask eligible = 0;
};

inline IndependentSetGraph make_independent_set_graph(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0 || n > kMaxIndependentSetVertices)
    {
        throw std::length_error("maximum_independent_set: vertex count must be in [0,46]");
    }
    IndependentSetGraph graph;
    graph.eligible = (IndependentSetMask{1} << n) - 1;
    for (const auto &[u, v] : edges)
    {
        const auto from = IndependentSetMask{1} << u;
        const auto to = IndependentSetMask{1} << v;
        graph.neighbors[u] |= to;
        graph.neighbors[v] |= from;
        if (u == v)
        {
            graph.eligible &= ~from;
        }
    }
    return graph;
}

template <class Score>
Score add_independent_set_scores(Score a, Score b)
{
    if constexpr (std::is_same_v<Score, std::uint64_t>)
    {
        return a <= kIndependentSetOverflow - b ? a + b : kIndependentSetOverflow;
    }
    else
    {
        return static_cast<Score>(a + b);
    }
}

template <class Score>
MaximumWeightIndependentSetResult solve_independent_set(
    const IndependentSetGraph &graph, const std::array<Score, kMaxIndependentSetVertices> &weights)
{
    std::array<int, kMaxIndependentSetVertices> vertices{};
    std::array<int, kMaxIndependentSetVertices> indices{};
    IndependentSetMask selected = 0;
    Score base = 0;
    int count = 0;
    for (auto remaining = graph.eligible; remaining != 0; remaining &= remaining - 1)
    {
        const int v = std::countr_zero(remaining);
        if ((graph.neighbors[v] & graph.eligible) == 0)
        {
            // 候補同士で隣接しない正の重みの頂点は必ず採用できる。
            selected |= IndependentSetMask{1} << v;
            base = add_independent_set_scores(base, weights[v]);
        }
        else
        {
            indices[v] = count;
            vertices[count++] = v;
        }
    }

    std::array<IndependentSetMask, kMaxIndependentSetVertices> neighbors{};
    for (int i = 0; i < count; ++i)
    {
        for (auto rest = graph.neighbors[vertices[i]] & graph.eligible; rest != 0; rest &= rest - 1)
        {
            neighbors[i] |= IndependentSetMask{1} << indices[std::countr_zero(rest)];
        }
    }

    using HalfMask = std::uint32_t;
    const int right_count = count / 2;
    const int left_count = count - right_count;
    const auto left_full = (HalfMask{1} << left_count) - 1;
    const auto right_full = (HalfMask{1} << right_count) - 1;
    std::array<HalfMask, kMaxIndependentSetVertices> left_neighbors{};
    std::array<HalfMask, kMaxIndependentSetVertices> right_neighbors{};
    for (int i = 0; i < count; ++i)
    {
        left_neighbors[i] = static_cast<HalfMask>(neighbors[i]) & left_full;
        right_neighbors[i] = static_cast<HalfMask>(neighbors[i] >> left_count);
    }

    // best_right[mask]は右側でmaskに含まれる頂点だけ使った場合の最適値。
    std::vector<Score> best_right(static_cast<std::size_t>(right_full) + 1);
    for (HalfMask mask = 1; mask <= right_full; ++mask)
    {
        const int v = left_count + std::countr_zero(mask);
        const auto without = mask & (mask - 1);
        const auto take = add_independent_set_scores(weights[vertices[v]], best_right[without & ~right_neighbors[v]]);
        best_right[mask] = std::max(best_right[without], take);
    }

    Score best = 0;
    HalfMask best_left = 0;
    HalfMask allowed_right = 0;
    const auto enumerate = [&](auto &&self, HalfMask candidates, HalfMask right, Score score, HalfMask subset) -> void
    {
        const auto total = add_independent_set_scores(score, best_right[right]);
        if (total > best)
        {
            best = total;
            best_left = subset;
            allowed_right = right;
        }
        while (candidates != 0)
        {
            const int u = std::countr_zero(candidates);
            const auto bit = HalfMask{1} << u;
            candidates ^= bit;
            const auto next_score = add_independent_set_scores(score, weights[vertices[u]]);
            self(self, candidates & ~left_neighbors[u], right & ~right_neighbors[u], next_score, subset | bit);
        }
    };
    enumerate(enumerate, left_full, right_full, base, 0);

    if constexpr (std::is_same_v<Score, std::uint64_t>)
    {
        if (best == kIndependentSetOverflow)
        {
            throw std::overflow_error("maximum_weight_independent_set: total weight does not fit int64_t");
        }
    }
    while (best_left != 0)
    {
        selected |= IndependentSetMask{1} << vertices[std::countr_zero(best_left)];
        best_left &= best_left - 1;
    }
    while (allowed_right != 0)
    {
        const int v = left_count + std::countr_zero(allowed_right);
        const auto without = allowed_right & (allowed_right - 1);
        if (best_right[allowed_right] != best_right[without])
        {
            selected |= IndependentSetMask{1} << vertices[v];
            allowed_right = without & ~right_neighbors[v];
        }
        else
        {
            allowed_right = without;
        }
    }
    MaximumWeightIndependentSetResult result{static_cast<std::int64_t>(best), {}};
    for (; selected != 0; selected &= selected - 1)
    {
        result.vertices.push_back(std::countr_zero(selected));
    }
    return result;
}
} // namespace detail

inline std::vector<int> maximum_independent_set(int n, std::span<const std::pair<int, int>> edges)
{
    const auto graph = detail::make_independent_set_graph(n, edges);
    std::array<std::uint8_t, kMaxIndependentSetVertices> weights;
    weights.fill(1);
    return detail::solve_independent_set(graph, weights).vertices;
}

inline MaximumWeightIndependentSetResult maximum_weight_independent_set(
    std::span<const std::int64_t> weights, std::span<const std::pair<int, int>> edges)
{
    if (weights.size() > kMaxIndependentSetVertices)
    {
        throw std::length_error("maximum_weight_independent_set: vertex count must be in [0,46]");
    }
    auto graph = detail::make_independent_set_graph(static_cast<int>(weights.size()), edges);
    std::array<std::uint64_t, kMaxIndependentSetVertices> positive_weights{};
    for (std::size_t v = 0; v < weights.size(); ++v)
    {
        if (weights[v] <= 0)
        {
            graph.eligible &= ~(detail::IndependentSetMask{1} << v);
        }
        else
        {
            positive_weights[v] = static_cast<std::uint64_t>(weights[v]);
        }
    }
    return detail::solve_independent_set(graph, positive_weights);
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
