// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Forest_Independent_Set.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/forest_independent_set.md
//
// API一覧
// ForestIndependentSet(n, edges): 0 始まりの無向森を作る。閉路、自己辺、多重辺は例外。
// count<T>(weights={}): 両端を同時に選ぶ辺がない頂点集合の個数。重みがあれば状態ごとの積を総和。
//   weights[v] = ForestIndependentSetWeight<T>{excluded,included}。空の森は1通り。
// count_by_vertex<T>(weights={}): 各頂点を選ばない・選ぶ場合の、森全体の重み付き個数を返す。
//   T は 0,1 から構築でき、可換な加算・乗算ができる型。除算を使わない。
// maximum_weight<T>(weights): 頂点重みの和を最大化する集合を {weight, vertices} で返す。空集合も可。
//   T は 0、加算、大小比較が必要。演算結果が T に収まること。すべて O(n+1) 時間・領域、非再帰。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FOREST_INDEPENDENT_SET_DEFINED
#define SIRKIT_FOREST_INDEPENDENT_SET_DEFINED

template <class T>
struct ForestIndependentSetWeight
{
    T excluded;
    T included;
};

template <class T>
struct ForestIndependentSetOptimum
{
    T weight;
    std::vector<int> vertices;
};

class ForestIndependentSet
{
public:
    ForestIndependentSet(int n, std::span<const std::pair<int, int>> edges)
    {
        if (n < 0)
        {
            throw std::length_error("ForestIndependentSet: negative vertex count");
        }
        if (edges.size() > static_cast<std::size_t>(n))
        {
            throw std::invalid_argument("ForestIndependentSet: too many edges");
        }
        std::vector<std::vector<int>> graph(n);
        for (const auto &[u, v] : edges)
        {
            if (u < 0 || u >= n || v < 0 || v >= n)
            {
                throw std::out_of_range("ForestIndependentSet: edge endpoint");
            }
            graph[u].push_back(v);
            graph[v].push_back(u);
        }

        constexpr int kUnvisited = -2;
        parent_.assign(n, kUnvisited);
        order_.reserve(n);
        children_.resize(n);
        for (int root = 0; root < n; ++root)
        {
            if (parent_[root] != kUnvisited)
            {
                continue;
            }
            parent_[root] = -1;
            roots_.push_back(root);
            const auto start = order_.size();
            order_.push_back(root);
            for (std::size_t i = start; i < order_.size(); ++i)
            {
                const int v = order_[i];
                for (const int w : graph[v])
                {
                    if (w == parent_[v])
                    {
                        continue;
                    }
                    if (parent_[w] != kUnvisited)
                    {
                        throw std::invalid_argument("ForestIndependentSet: cycle");
                    }
                    parent_[w] = v;
                    children_[v].push_back(w);
                    order_.push_back(w);
                }
            }
        }
    }

    template <class T>
    [[nodiscard]] T count(std::span<const ForestIndependentSetWeight<T>> weights = {}) const
    {
        const auto subtree = count_subtrees(weights);
        T total{1};
        for (const int root : roots_)
        {
            total = total * (subtree[root].excluded + subtree[root].included);
        }
        return total;
    }

    template <class T>
    [[nodiscard]] std::vector<ForestIndependentSetWeight<T>>
    count_by_vertex(std::span<const ForestIndependentSetWeight<T>> weights = {}) const
    {
        using Pair = ForestIndependentSetWeight<T>;
        const auto subtree = count_subtrees(weights);
        const Pair one{T{1}, T{1}};
        std::vector<Pair> result(order_.size(), one), prefixes(order_.size(), one);
        T previous{1};
        for (const int root : roots_)
        {
            result[root] = {previous, previous};
            previous = previous * (subtree[root].excluded + subtree[root].included);
        }
        T after{1};
        for (auto it = roots_.rbegin(); it != roots_.rend(); ++it)
        {
            const int root = *it;
            result[root] = {result[root].excluded * after, result[root].included * after};
            after = after * (subtree[root].excluded + subtree[root].included);
        }

        for (const int v : order_)
        {
            const Pair vertex = weights.empty() ? one : weights[v];
            Pair before{result[v].excluded * vertex.excluded, result[v].included * vertex.included};
            for (const int w : children_[v])
            {
                prefixes[w] = before;
                include_child(before, subtree[w]);
            }
            result[v] = before;

            Pair suffix = one;
            for (auto it = children_[v].rbegin(); it != children_[v].rend(); ++it)
            {
                const int w = *it;
                const T without_v = prefixes[w].excluded * suffix.excluded;
                const T with_v = prefixes[w].included * suffix.included;
                result[w] = {without_v + with_v, without_v};
                include_child(suffix, subtree[w]);
            }
        }
        return result;
    }

    template <class T>
    [[nodiscard]] ForestIndependentSetOptimum<T> maximum_weight(std::span<const T> weights) const
    {
        if (weights.size() != order_.size())
        {
            throw std::invalid_argument("ForestIndependentSet: wrong number of vertex weights");
        }
        using Pair = ForestIndependentSetWeight<T>;
        std::vector<Pair> subtree(order_.size(), {T{0}, T{0}});
        for (auto it = order_.rbegin(); it != order_.rend(); ++it)
        {
            const int v = *it;
            subtree[v].included = weights[v];
            for (const int w : children_[v])
            {
                subtree[v].excluded = subtree[v].excluded + std::max(subtree[w].excluded, subtree[w].included);
                subtree[v].included = subtree[v].included + subtree[w].excluded;
            }
        }

        ForestIndependentSetOptimum<T> result{T{0}, {}};
        std::vector<bool> picked(order_.size());
        for (const int v : order_)
        {
            const int parent = parent_[v];
            if ((parent == -1 || !picked[parent]) && subtree[v].excluded < subtree[v].included)
            {
                picked[v] = true;
                result.weight = result.weight + weights[v];
                result.vertices.push_back(v);
            }
        }
        return result;
    }

private:
    template <class T>
    static void include_child(ForestIndependentSetWeight<T> &parent, const ForestIndependentSetWeight<T> &child)
    {
        parent.excluded = parent.excluded * (child.excluded + child.included);
        parent.included = parent.included * child.excluded;
    }

    template <class T>
    std::vector<ForestIndependentSetWeight<T>> count_subtrees(std::span<const ForestIndependentSetWeight<T>> weights) const
    {
        using Pair = ForestIndependentSetWeight<T>;
        if (!weights.empty() && weights.size() != order_.size())
        {
            throw std::invalid_argument("ForestIndependentSet: wrong number of counting weights");
        }
        const Pair one{T{1}, T{1}};
        std::vector<Pair> subtree(order_.size(), one);
        for (auto it = order_.rbegin(); it != order_.rend(); ++it)
        {
            const int v = *it;
            auto &current = subtree[v];
            if (!weights.empty())
            {
                current = weights[v];
            }
            for (const int w : children_[v])
            {
                include_child(current, subtree[w]);
            }
        }
        return subtree;
    }

    std::vector<int> parent_, order_, roots_;
    std::vector<std::vector<int>> children_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
