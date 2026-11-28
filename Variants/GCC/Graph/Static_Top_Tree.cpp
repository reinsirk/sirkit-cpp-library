// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Static_Top_Tree.cpp
// 参考（静的トップ木と頂点数での重み付け）: https://atcoder.jp/contests/abc351/editorial/9899
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/static_top_tree.md
//
// API一覧
// StaticTopTree<Policy>(values, edges, root=0, policy={}): 0始まりの空でない無向木を構築する。
// Policy::Vertex/Path/Point: 頂点値、根に向かうパスの要約、枝の要約の型。
// policy.vertex(value, branches): 頂点と横の枝をパスにする。枝がなければbranchesはnullptr。
// policy.project(path): 完成したパスを親へつながる枝に変換する。
// policy.rake(left, right): 横の枝を結合する。結合則と交換則が必要。
// policy.compress(upper, lower): 上側と下側のパスを結合する。結合則が必要。
// 各コールバックはconstかつnoexcept。頂点値・要約のムーブ代入もnoexcept。
// size(): 頂点数。O(1)。set(v, value): 頂点値を置き換える。O(log(n+1))。
// all_prod(): 根を含むパス全体の要約へのconst参照。O(1)。必要ならproject相当の変換を呼ぶ。
// 各コールバックがO(1)なら、構築O(n log(n+1))時間、領域O(n)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STATIC_TOP_TREE_DEFINED
#define SIRKIT_STATIC_TOP_TREE_DEFINED

namespace detail
{
inline void check_static_top_tree_capacity(std::size_t n)
{
    // 頂点・重い辺・軽い辺を合わせて2n-1個。横の枝同士の結合を足しても3n未満。
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max() / 3))
    {
        throw std::length_error("StaticTopTree: too many vertices");
    }
}
}

template<class Policy>
class StaticTopTree
{
public:
    using Vertex = typename Policy::Vertex;
    using Path = typename Policy::Path;
    using Point = typename Policy::Point;
    using Edge = std::pair<int, int>;

    StaticTopTree(std::span<const Vertex> values, std::span<const Edge> edges, int root = 0, Policy policy = {})
        : policy_(std::move(policy))
    {
        const std::size_t n = values.size();
        detail::check_static_top_tree_capacity(n);
#ifndef SKIP_RUNTIME_CHECKS
        if (root < 0 || static_cast<std::size_t>(root) >= n)
        {
            throw std::out_of_range("StaticTopTree: invalid root");
        }
        if (edges.size() != n - 1)
        {
            throw std::invalid_argument("StaticTopTree: invalid edge count");
        }
#endif
        std::vector<std::size_t> offset(n + 1);
        for (const auto &[u, v] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (u < 0 || static_cast<std::size_t>(u) >= n || v < 0 || static_cast<std::size_t>(v) >= n || u == v)
            {
                throw std::invalid_argument("StaticTopTree: invalid endpoint");
            }
#endif
            ++offset[static_cast<std::size_t>(u) + 1];
            ++offset[static_cast<std::size_t>(v) + 1];
        }
        for (std::size_t i = 0; i < n; ++i)
        {
            offset[i + 1] += offset[i];
        }
        std::vector<int> adjacency(offset.back());
        auto next = offset;
        for (const auto &[u, v] : edges)
        {
            adjacency[next[u]++] = v;
            adjacency[next[v]++] = u;
        }

        std::vector<int> parent(n, -1), order{root};
        order.reserve(n);
        parent[root] = root;
        for (std::size_t i = 0; i < order.size(); ++i)
        {
            const int v = order[i];
            for (std::size_t j = offset[v]; j < offset[v + 1]; ++j)
            {
                const int u = adjacency[j];
                if (u == parent[v])
                {
                    continue;
                }
#ifndef SKIP_RUNTIME_CHECKS
                if (parent[u] != -1)
                {
                    throw std::invalid_argument("StaticTopTree: cycle");
                }
#endif
                parent[u] = v;
                order.push_back(u);
            }
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (order.size() != n)
        {
            throw std::invalid_argument("StaticTopTree: disconnected tree");
        }
#endif
        std::vector<int> weight(n, 1), heavy(n, -1);
        int heavy_edges = 0;
        for (std::size_t i = n; i > 1; --i)
        {
            const int v = order[i - 1], p = parent[v];
            weight[p] += weight[v];
            if (heavy[p] == -1)
            {
                ++heavy_edges;
                heavy[p] = v;
            }
            else if (weight[v] > weight[heavy[p]])
            {
                heavy[p] = v;
            }
        }

        vertices_ = std::vector<Vertex>(values.begin(), values.end());
        leaf_.resize(n);
        const std::size_t path_count = n + heavy_edges;
        const std::size_t light_edges = n - 1 - heavy_edges;
        paths_.reserve(path_count);
        points_.reserve(2 * light_edges);
        nodes_.reserve(path_count + 2 * light_edges);
        std::vector<int> path_top(n);
        std::vector<Weighted> branches, chain;
        for (std::size_t i = n; i > 0; --i)
        {
            const int top = order[i - 1];
            if (top != root && heavy[parent[top]] == top)
            {
                continue;
            }
            chain.clear();
            for (int v = top; v != -1; v = heavy[v])
            {
                branches.clear();
                for (std::size_t j = offset[v]; j < offset[v + 1]; ++j)
                {
                    const int u = adjacency[j];
                    if (u != parent[v] && u != heavy[v])
                    {
                        const int branch = Append(Kind::kProject, path_top[u], -1);
                        AppendWeighted(branches, branch, weight[u]);
                    }
                }
                const int side = branches.empty() ? -1 : Join(branches, 0, branches.size(), Kind::kRake);
                leaf_[v] = Append(Kind::kVertex, v, side);
                const int weight_below = heavy[v] == -1 ? 0 : weight[heavy[v]];
                AppendWeighted(chain, leaf_[v], weight[v] - weight_below);
            }
            path_top[top] = Join(chain, 0, chain.size(), Kind::kCompress);
        }
    }

    StaticTopTree(const StaticTopTree &) = default;
    StaticTopTree(StaticTopTree &&) noexcept = default;

    StaticTopTree &operator=(const StaticTopTree &other)
    {
        return *this = StaticTopTree(other);
    }

    StaticTopTree &operator=(StaticTopTree &&) noexcept = default;

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(vertices_.size());
    }

    void set(int v, Vertex value)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || v >= size())
        {
            throw std::out_of_range("StaticTopTree: invalid vertex");
        }
#endif
        vertices_[v] = std::move(value);
        for (int node = leaf_[v]; node != -1; node = nodes_[node].parent)
        {
            Recompute<false>(nodes_[node]);
        }
    }

    [[nodiscard]] const Path &all_prod() const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (paths_.empty())
        {
            throw std::logic_error("StaticTopTree: moved-from tree");
        }
#endif
        return paths_.back().value;
    }

private:
    static_assert(std::is_nothrow_move_constructible_v<Policy> && std::is_nothrow_move_assignable_v<Policy>);
    static_assert(std::is_nothrow_move_assignable_v<Vertex> &&
                  std::is_nothrow_move_assignable_v<Path> && std::is_nothrow_move_assignable_v<Point>);
    static_assert(noexcept(std::declval<const Policy &>().vertex(std::declval<const Vertex &>(), std::declval<const Point *>())));
    static_assert(noexcept(std::declval<const Policy &>().project(std::declval<const Path &>())));
    static_assert(noexcept(std::declval<const Policy &>().rake(std::declval<const Point &>(), std::declval<const Point &>())));
    static_assert(noexcept(std::declval<const Policy &>().compress(std::declval<const Path &>(), std::declval<const Path &>())));

    enum class Kind : unsigned char
    {
        kVertex,
        kProject,
        kRake,
        kCompress,
    };

    struct Cluster
    {
        int left, right, parent, state;
        Kind kind;
    };

    struct Weighted
    {
        int cluster, cumulative;
    };

    template<class State>
    struct Cell
    {
        State value;
    };

    [[no_unique_address]] Policy policy_;
    std::vector<Vertex> vertices_;
    std::vector<int> leaf_;
    std::vector<Cluster> nodes_;
    std::vector<Cell<Path>> paths_;
    std::vector<Cell<Point>> points_;

    static void AppendWeighted(std::vector<Weighted> &items, int cluster, int weight)
    {
        const int previous = items.empty() ? 0 : items.back().cumulative;
        items.push_back({cluster, previous + weight});
    }

    int Join(std::span<const Weighted> items, std::size_t first, std::size_t last, Kind kind)
    {
        if (last - first == 1)
        {
            return items[first].cluster;
        }
        // 中央を含む要素を除くと、左右とも元の重みの半分以下になる。
        const int base = first == 0 ? 0 : items[first - 1].cumulative;
        const int target = base + (items[last - 1].cumulative - base + 1) / 2;
        const auto median = std::lower_bound(items.begin() + first, items.begin() + last, target,
                                            [](const Weighted &item, int value) { return item.cumulative < value; });
        const auto middle = static_cast<std::size_t>(median - items.begin());
        int cluster = median->cluster;
        if (first != middle)
        {
            const int left = Join(items, first, middle, kind);
            cluster = Append(kind, left, cluster);
        }
        if (middle + 1 != last)
        {
            const int right = Join(items, middle + 1, last, kind);
            cluster = Append(kind, cluster, right);
        }
        return cluster;
    }

    int Append(Kind kind, int left, int right)
    {
        const int index = static_cast<int>(nodes_.size());
        const auto state = static_cast<int>(kind == Kind::kProject || kind == Kind::kRake ? points_.size() : paths_.size());
        nodes_.push_back({left, right, -1, state, kind});
        if (kind != Kind::kVertex)
        {
            nodes_[left].parent = index;
        }
        if (right != -1)
        {
            nodes_[right].parent = index;
        }
        Recompute<true>(nodes_[index]);
        return index;
    }

    template<bool AppendState, class State>
    static void Store(std::vector<Cell<State>> &states, int index, State value) noexcept(!AppendState)
    {
        if constexpr (AppendState)
        {
            states.push_back({std::move(value)});
        }
        else
        {
            states[index].value = std::move(value);
        }
    }

    const Path &PathState(int cluster) const noexcept
    {
        return paths_[nodes_[cluster].state].value;
    }

    const Point &PointState(int cluster) const noexcept
    {
        return points_[nodes_[cluster].state].value;
    }

    template<bool AppendState>
    void Recompute(const Cluster &cluster) noexcept(!AppendState)
    {
        const Policy &policy = policy_;
        if (cluster.kind == Kind::kVertex)
        {
            const Point *side = cluster.right == -1 ? nullptr : &PointState(cluster.right);
            Store<AppendState>(paths_, cluster.state, policy.vertex(std::as_const(vertices_)[cluster.left], side));
        }
        else if (cluster.kind == Kind::kProject)
        {
            Store<AppendState>(points_, cluster.state, policy.project(PathState(cluster.left)));
        }
        else if (cluster.kind == Kind::kRake)
        {
            Store<AppendState>(points_, cluster.state, policy.rake(PointState(cluster.left), PointState(cluster.right)));
        }
        else
        {
            Store<AppendState>(paths_, cluster.state, policy.compress(PathState(cluster.left), PathState(cluster.right)));
        }
    }
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
