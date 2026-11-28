// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Core_Decomposition.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/core_decomposition.md
//
// API一覧
// core_decomposition(n, edges): 0 始まりの無向グラフを k-core 分解する。O(n+m+1) 時間。
// weighted_core_decomposition<T>(n, edges, weights): 次数を残っている隣接頂点の重みの和に置き換える。
//   T は bool 以外の整数型。非負重みを頂点数分渡し、隣接重みの総和は T に収まること。
//   O((n+m+1) log(n+m+2)) 時間。どちらも O(n+m+1) 領域。多重辺可、自己辺不可。
// CoreDecompositionResult<T>{number, order, degeneracy}:
//   number[v] は v が属する最大の k-core の k。order は削除順、degeneracy は最大のコア番号。
//   この順での各削除コストは number[v] 以下。重み付きでは最大削除コストを最小にする。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <limits>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CORE_DECOMPOSITION_DEFINED
#define SIRKIT_CORE_DECOMPOSITION_DEFINED

template <class T>
struct CoreDecompositionResult
{
    std::vector<T> number;
    std::vector<int> order;
    T degeneracy;
};

namespace detail
{
inline std::vector<std::vector<int>> core_decomposition_graph(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("core_decomposition: negative vertex count");
    }
    std::vector<std::vector<int>> graph(n);
    for (const auto &[u, v] : edges)
    {
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("core_decomposition: edge endpoint");
        }
        if (u == v)
        {
            throw std::invalid_argument("core_decomposition: self-loop");
        }
        graph[u].push_back(v);
        graph[v].push_back(u);
    }
    return graph;
}

template <std::integral T>
void add_core_weight(T &total, T weight)
{
    if (weight > std::numeric_limits<T>::max() - total)
    {
        throw std::overflow_error("weighted_core_decomposition: neighbor sum exceeds result type");
    }
    total += weight;
}
} // namespace detail

[[nodiscard]] inline CoreDecompositionResult<std::size_t> core_decomposition(
    int n, std::span<const std::pair<int, int>> edges)
{
    const auto graph = detail::core_decomposition_graph(n, edges);
    CoreDecompositionResult<std::size_t> result{std::vector<std::size_t>(n), std::vector<int>(n), 0};
    std::size_t maximum = 0;
    for (int v = 0; v < n; ++v)
    {
        result.number[v] = graph[v].size();
        maximum = std::max(maximum, result.number[v]);
    }
    std::vector<std::size_t> first(maximum + 1);
    for (const auto d : result.number)
    {
        ++first[d];
    }
    std::size_t start = 0;
    for (auto &bucket : first)
    {
        const auto count = bucket;
        bucket = start;
        start += count;
    }
    auto next = first;
    std::vector<std::size_t> position(n);
    for (int v = 0; v < n; ++v)
    {
        position[v] = next[result.number[v]]++;
        result.order[position[v]] = v;
    }
    for (const int v : result.order)
    {
        const auto threshold = result.number[v];
        result.degeneracy = threshold;
        for (const int w : graph[v])
        {
            auto &degree = result.number[w];
            if (degree > threshold)
            {
                const auto front = first[degree]++;
                const auto at = position[w];
                const int other = result.order[front];
                std::swap(result.order[front], result.order[at]);
                position[w] = front;
                position[other] = at;
                --degree;
            }
        }
    }
    return result;
}

template <std::integral T>
    requires(!std::same_as<T, bool>)
[[nodiscard]] CoreDecompositionResult<T> weighted_core_decomposition(
    int n, std::span<const std::pair<int, int>> edges, std::span<const T> weights)
{
    const auto graph = detail::core_decomposition_graph(n, edges);
    if (weights.size() != graph.size())
    {
        throw std::invalid_argument("weighted_core_decomposition: wrong number of weights");
    }
    if constexpr (std::signed_integral<T>)
    {
        for (const T w : weights)
        {
            if (w < 0)
            {
                throw std::invalid_argument("weighted_core_decomposition: negative vertex weight");
            }
        }
    }

    using Candidate = std::pair<T, int>;
    std::vector<Candidate> initial;
    initial.reserve(n);
    std::vector<T> score(n);
    for (int v = 0; v < n; ++v)
    {
        for (const int w : graph[v])
        {
            detail::add_core_weight(score[v], weights[w]);
        }
        initial.emplace_back(score[v], v);
    }
    std::priority_queue<Candidate, std::vector<Candidate>, std::greater<Candidate>> queue(
        std::greater<Candidate>{}, std::move(initial));
    CoreDecompositionResult<T> result{std::vector<T>(n), {}, T{0}};
    result.order.reserve(n);
    std::vector<bool> removed(n);
    while (!queue.empty())
    {
        const auto [cost, v] = queue.top();
        queue.pop();
        if (removed[v])
        {
            continue;
        }
        removed[v] = true;
        result.degeneracy = std::max(result.degeneracy, cost);
        result.number[v] = result.degeneracy;
        result.order.push_back(v);
        for (const int w : graph[v])
        {
            if (!removed[w])
            {
                score[w] -= weights[v];
                queue.emplace(score[w], w);
            }
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
