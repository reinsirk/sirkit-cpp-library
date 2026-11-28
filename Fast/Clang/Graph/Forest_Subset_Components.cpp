// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Forest_Subset_Components.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/forest_subset_components.md
//
// API一覧
// ForestSubsetWeight<T>{omit=1,take=1}: 頂点を選ばないとき・選ぶときの係数。
// forest_subset_components<T>(n,edges,weights={}): 森の頂点部分集合を、誘導部分グラフの成分数ごとに集計。
//   頂点は0始まり。各解の重みは頂点の係数の積。weightsを省くと各成分数の個数。
//   戻り値の添字kは成分数k。0番は空集合。T{} は加法単位元、T{1} は乗法単位元。可換な加算と乗算を使う。
//   木DPで根を選ぶ場合・選ばない場合を持つ。O((n+1)²) 時間、O(n+1) 領域。再帰を使わない。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FOREST_SUBSET_COMPONENTS_DEFINED
#define SIRKIT_FOREST_SUBSET_COMPONENTS_DEFINED

template <class T>
struct ForestSubsetWeight
{
    T omit{1}, take{1};
};

namespace detail
{
template <class T>
using ForestSubsetRows = std::vector<std::array<T, 2>>;

template <class T>
ForestSubsetRows<T> merge_forest_subset_rows(const ForestSubsetRows<T> &parent, const ForestSubsetRows<T> &child)
{
    ForestSubsetRows<T> result(parent.size() + child.size());
    for (std::size_t i = 0; i < parent.size(); ++i)
    {
        const auto &[omit_parent, take_parent] = parent[i];
        for (std::size_t j = 0; j < child.size(); ++j)
        {
            const auto &[omit_child, take_child] = child[j];
            const auto count = i + j;
            result[count][0] = result[count][0] + omit_parent * omit_child;
            result[count + 1][0] = result[count + 1][0] + omit_parent * take_child;
            result[count][1] = result[count][1] + take_parent * (omit_child + take_child);
        }
    }
    return result;
}
} // namespace detail

template <class T>
[[nodiscard]] std::vector<T> forest_subset_components(
    int n, std::span<const std::pair<int, int>> edges, std::span<const ForestSubsetWeight<T>> weights = {})
{
    if (n < 0)
    {
        throw std::length_error("forest_subset_components: vertex count");
    }
    if (!weights.empty() && weights.size() != static_cast<std::size_t>(n))
    {
        throw std::invalid_argument("forest_subset_components: weight count");
    }
    std::vector<std::vector<int>> graph(n);
    for (const auto &[u, v] : edges)
    {
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("forest_subset_components: edge endpoint");
        }
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    constexpr int kUndiscovered = -2;
    std::vector<int> parent(n, kUndiscovered), order;
    order.reserve(n);
    for (int root = 0; root < n; ++root)
    {
        if (parent[root] != kUndiscovered)
        {
            continue;
        }
        const auto start = order.size();
        order.push_back(root);
        parent[root] = -1;
        for (std::size_t i = start; i < order.size(); ++i)
        {
            const int u = order[i];
            for (const int v : graph[u])
            {
                if (v == parent[u])
                {
                    continue;
                }
                if (parent[v] != kUndiscovered)
                {
                    throw std::invalid_argument("forest_subset_components: graph contains a cycle");
                }
                parent[v] = u;
                order.push_back(v);
            }
        }
    }

    // 行iは根を除く場合にi成分、根を含む場合にi+1成分を表す。
    detail::ForestSubsetRows<T> aggregate{{T{1}, T{}}};
    std::vector<detail::ForestSubsetRows<T>> rows(n);
    for (auto it = order.rbegin(); it != order.rend(); ++it)
    {
        const int vertex = *it;
        const auto weight = weights.empty() ? ForestSubsetWeight<T>{} : weights[vertex];
        detail::ForestSubsetRows<T> state{{weight.omit, weight.take}};
        for (const int child : graph[vertex])
        {
            if (parent[child] == vertex)
            {
                const auto completed = std::move(rows[child]);
                state = detail::merge_forest_subset_rows(state, completed);
            }
        }
        if (parent[vertex] == -1)
        {
            aggregate = detail::merge_forest_subset_rows(aggregate, state);
        }
        else
        {
            rows[vertex] = std::move(state);
        }
    }

    std::vector<T> answer;
    answer.reserve(aggregate.size());
    for (const auto &item : aggregate)
    {
        answer.push_back(item[0]);
    }
    return answer;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
