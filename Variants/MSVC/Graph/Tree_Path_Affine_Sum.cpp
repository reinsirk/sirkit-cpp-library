// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Graph/Tree_Path_Affine_Sum.cpp
// 参考（静的Top Treeによる木の集約）: https://atcoder.jp/contests/abc351/editorial/9910
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_path_affine_sum.md
//
// API一覧
// TreePathAffineSum<T>(values, edges): 頂点値と辺のアフィン写像を持つ無向木を構築する。
// Affine{mul, add}: x -> mul*x + add。Edge{u, v, toward_u, toward_v}: 両方向の写像を指定する辺。
// size(): 頂点数、O(1)。空の木も扱う。頂点・辺番号は0始まりで、辺番号は入力順。
// set_vertex(v, value): 頂点値を置き換える。set_edge(i, f) / set_edge(i, toward_u, toward_v): 辺の写像を置き換える。
// sum_from(root): 全頂点の値を各パスに沿ってrootへ写した総和。all_prod(): 根0での総和、空なら0。O(1)。
// 構築O(n log(n+1))時間・O(n)領域、更新と任意の根での取得O(log(n+1))時間。Tの演算はO(1)とする。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_PATH_AFFINE_SUM_DEFINED
#define SIRKIT_TREE_PATH_AFFINE_SUM_DEFINED

namespace detail
{
inline void check_tree_path_affine_capacity(std::size_t n)
{
    // 全体で5n個未満のクラスタを作り、番号をintに保持する。
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max() / 5))
    {
        throw std::length_error("TreePathAffineSum: too many vertices");
    }
}
}

template<class T>
class TreePathAffineSum
{
public:
    struct Affine
    {
        T mul{1};
        T add{0};
    };

    struct Edge
    {
        int u;
        int v;
        Affine toward_u;
        Affine toward_v;
    };

    TreePathAffineSum(std::span<const T> values, std::span<const Edge> edges)
    {
        const auto n = values.size();
        detail::check_tree_path_affine_capacity(n);
#ifndef SKIP_RUNTIME_CHECKS
        if (edges.size() != n - (n != 0))
        {
            throw std::invalid_argument("TreePathAffineSum: invalid edge count");
        }
#endif
        if (n == 0)
        {
            return;
        }

        struct Arc
        {
            int to;
            int edge;
        };
        std::vector<std::size_t> begin(n + 1);
        for (const auto &edge : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (edge.u < 0 || static_cast<std::size_t>(edge.u) >= n || edge.v < 0 ||
                static_cast<std::size_t>(edge.v) >= n || edge.u == edge.v)
            {
                throw std::invalid_argument("TreePathAffineSum: invalid endpoint");
            }
#endif
            ++begin[static_cast<std::size_t>(edge.u) + 1];
            ++begin[static_cast<std::size_t>(edge.v) + 1];
        }
        for (std::size_t v = 0; v < n; ++v)
        {
            begin[v + 1] += begin[v];
        }
        std::vector<Arc> adjacency(begin.back());
        auto next = begin;
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            const auto &edge = edges[i];
            const auto id = static_cast<int>(i);
            adjacency[next[edge.u]++] = {edge.v, id};
            adjacency[next[edge.v]++] = {edge.u, id};
        }

        std::vector<int> parent(n, -1), incoming(n, -1), order{0};
        order.reserve(n);
        parent[0] = 0;
        for (std::size_t i = 0; i < order.size(); ++i)
        {
            const int v = order[i];
            for (std::size_t j = begin[v]; j != begin[static_cast<std::size_t>(v) + 1]; ++j)
            {
                const auto [to, edge] = adjacency[j];
                if (edge == incoming[v])
                {
                    continue;
                }
#ifndef SKIP_RUNTIME_CHECKS
                if (parent[to] != -1)
                {
                    throw std::invalid_argument("TreePathAffineSum: cycle");
                }
#endif
                parent[to] = v;
                incoming[to] = edge;
                order.push_back(to);
            }
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (order.size() != n)
        {
            throw std::invalid_argument("TreePathAffineSum: disconnected tree");
        }
#endif
        std::vector<int> subtree(n, 1), heavy(n, -1);
        for (std::size_t i = n; i > 1; --i)
        {
            const int v = order[i - 1], p = parent[v];
            subtree[p] += subtree[v];
            if (heavy[p] == -1 || subtree[heavy[p]] < subtree[v])
            {
                heavy[p] = v;
            }
        }

        clusters_.reserve(5 * n);
        vertex_leaf_.reserve(n);
        for (const auto &value : values)
        {
            vertex_leaf_.push_back(static_cast<int>(clusters_.size()));
            clusters_.emplace_back();
            auto &cluster = clusters_.back();
            cluster.count = 1;
            cluster.top_sum = cluster.bottom_sum = value;
        }
        edge_leaf_.resize(edges.size());
        edge_reversed_.resize(edges.size());
        for (std::size_t i = 1; i < n; ++i)
        {
            const int v = order[i], id = incoming[v];
            const auto &edge = edges[id];
            edge_leaf_[id] = static_cast<int>(clusters_.size());
            edge_reversed_[id] = edge.u == v;
            clusters_.emplace_back();
            assign_edge(id, edge.toward_u, edge.toward_v);
        }

        std::vector<int> complete(n, -1), points, segments, prefix;
        for (auto it = order.rbegin(); it != order.rend(); ++it)
        {
            const int head = *it;
            if (head != 0 && heavy[parent[head]] == head)
            {
                continue;
            }
            segments.clear();
            for (int v = head; v != -1; v = heavy[v])
            {
                points.clear();
                points.push_back(vertex_leaf_[v]);
                for (std::size_t i = begin[v]; i != begin[static_cast<std::size_t>(v) + 1]; ++i)
                {
                    const auto [to, edge] = adjacency[i];
                    if (parent[to] != v || to == heavy[v])
                    {
                        continue;
                    }
                    // 軽い子の全体を根側だけに接続する部分木へ変える。
                    const int child_path = join(edge_leaf_[edge], complete[to]);
                    points.push_back(join(child_path, -1));
                }
                int segment = join_weighted(points, prefix);
                if (heavy[v] != -1)
                {
                    segment = join(segment, edge_leaf_[incoming[heavy[v]]]);
                }
                segments.push_back(segment);
            }
            complete[head] = join_weighted(segments, prefix);
        }
        root_cluster_ = complete[0];
    }

    TreePathAffineSum(const TreePathAffineSum &) = default;

    TreePathAffineSum(TreePathAffineSum &&other) noexcept
    {
        swap_storage(other);
    }

    TreePathAffineSum &operator=(TreePathAffineSum other) noexcept
    {
        swap_storage(other);
        return *this;
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(vertex_leaf_.size());
    }

    void set_vertex(int v, const T &value)
    {
#ifndef SKIP_RUNTIME_CHECKS
        check_vertex(v);
#endif
        const int index = vertex_leaf_[v];
        auto &leaf = clusters_[index];
        leaf.top_sum = leaf.bottom_sum = value;
        refresh_ancestors(index);
    }

    void set_edge(int id, const Affine &transform)
    {
        set_edge(id, transform, transform);
    }

    void set_edge(int id, const Affine &toward_u, const Affine &toward_v)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (id < 0 || static_cast<std::size_t>(id) >= edge_leaf_.size())
        {
            throw std::out_of_range("TreePathAffineSum: edge index");
        }
#endif
        assign_edge(id, toward_u, toward_v);
        refresh_ancestors(edge_leaf_[id]);
    }

    [[nodiscard]] T all_prod() const
    {
        return root_cluster_ == -1 ? T{0} : clusters_[root_cluster_].top_sum;
    }

    [[nodiscard]] T sum_from(int root) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        check_vertex(root);
#endif
        int index = vertex_leaf_[root];
        T answer = clusters_[index].top_sum;
        Affine from_top, from_bottom;
        while (clusters_[index].parent != -1)
        {
            const int p = clusters_[index].parent;
            const auto &parent = clusters_[p];
            if (parent.right == -1)
            {
                from_bottom = from_top;
            }
            else if (parent.left == index)
            {
                const auto &other = clusters_[parent.right];
                answer = answer + apply(from_bottom, other.top_sum, other.count);
                from_bottom = compose(from_bottom, other.top_from_bottom);
            }
            else
            {
                const auto &other = clusters_[parent.left];
                answer = answer + apply(from_top, other.bottom_sum, other.count);
                from_top = compose(from_top, other.bottom_from_top);
            }
            index = p;
        }
        return answer;
    }

private:
    struct Cluster
    {
        int left = -1;
        int right = -1;
        int parent = -1;
        int count = 0;
        Affine top_from_bottom, bottom_from_top;
        T top_sum{0};
        T bottom_sum{0};
    };

    static T apply(const Affine &f, const T &sum, int count)
    {
        return f.mul * sum + f.add * static_cast<T>(count);
    }

    static Affine compose(const Affine &outer, const Affine &inner)
    {
        return {outer.mul * inner.mul, outer.mul * inner.add + outer.add};
    }

    void swap_storage(TreePathAffineSum &other) noexcept
    {
        clusters_.swap(other.clusters_);
        vertex_leaf_.swap(other.vertex_leaf_);
        edge_leaf_.swap(other.edge_leaf_);
        edge_reversed_.swap(other.edge_reversed_);
        std::swap(root_cluster_, other.root_cluster_);
    }

#ifndef SKIP_RUNTIME_CHECKS
    void check_vertex(int v) const
    {
        if (v < 0 || v >= size())
        {
            throw std::out_of_range("TreePathAffineSum: vertex index");
        }
    }
#endif

    void assign_edge(int id, const Affine &toward_u, const Affine &toward_v)
    {
        auto &cluster = clusters_[edge_leaf_[id]];
        const bool reversed = edge_reversed_[id] != 0;
        cluster.top_from_bottom = reversed ? toward_v : toward_u;
        cluster.bottom_from_top = reversed ? toward_u : toward_v;
    }

    void pull(int index)
    {
        auto &node = clusters_[index];
        const auto &left = clusters_[node.left];
        if (node.right == -1)
        {
            node.count = left.count;
            node.top_sum = node.bottom_sum = left.top_sum;
            return;
        }
        const auto &right = clusters_[node.right];
        node.count = left.count + right.count;
        node.top_sum = left.top_sum + apply(left.top_from_bottom, right.top_sum, right.count);
        node.bottom_sum = right.bottom_sum + apply(right.bottom_from_top, left.bottom_sum, left.count);
        node.top_from_bottom = compose(left.top_from_bottom, right.top_from_bottom);
        node.bottom_from_top = compose(right.bottom_from_top, left.bottom_from_top);
    }

    void refresh_ancestors(int index)
    {
        for (int p = clusters_[index].parent; p != -1; p = clusters_[p].parent)
        {
            pull(p);
        }
    }

    int join(int left, int right)
    {
        const auto index = static_cast<int>(clusters_.size());
        clusters_.emplace_back();
        clusters_[index].left = left;
        clusters_[index].right = right;
        clusters_[left].parent = index;
        if (right != -1)
        {
            clusters_[right].parent = index;
        }
        pull(index);
        return index;
    }

    int join_interval(std::span<const int> parts, std::span<const int> prefix, int first, int last)
    {
        if (last - first == 1)
        {
            return parts[first];
        }
        // 個数でなく含まれる頂点数の差が小さくなる場所で切る。
        const auto target = (std::int64_t{prefix[first]} + prefix[last] + 1) / 2;
        int cut = static_cast<int>(std::lower_bound(prefix.begin() + first + 1, prefix.begin() + last, target) - prefix.begin());
        if (cut == last || (cut > first + 1 && std::int64_t{prefix[cut - 1]} + prefix[cut] >
                                                   std::int64_t{prefix[first]} + prefix[last]))
        {
            --cut;
        }
        const int left = join_interval(parts, prefix, first, cut);
        const int right = join_interval(parts, prefix, cut, last);
        return join(left, right);
    }

    int join_weighted(std::span<const int> parts, std::vector<int> &prefix)
    {
        prefix.clear();
        prefix.push_back(0);
        for (const int part : parts)
        {
            prefix.push_back(prefix.back() + clusters_[part].count);
        }
        return join_interval(parts, prefix, 0, static_cast<int>(parts.size()));
    }

    std::vector<Cluster> clusters_;
    std::vector<int> vertex_leaf_, edge_leaf_;
    std::vector<unsigned char> edge_reversed_;
    int root_cluster_ = -1;
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
