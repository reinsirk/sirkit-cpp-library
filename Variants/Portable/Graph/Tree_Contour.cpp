// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Tree_Contour.cpp
// 参考（Fenwick Tree）: https://doi.org/10.1002/spe.4380240306
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_contour.md
//
// API一覧
// TreeContourIndex(n, edges): 辺{u,v}の長さを1とする無向木を前処理する。
// TreeContourIndex::weighted(n, edges): TreeContourEdge{from,to,length}の非負int64_t長を用いる。
// 前処理はO(1+n log²(n+1))時間・O(1+n log(n+1))領域。size()は頂点数をO(1)で返す。
// VertexAddContourSum<T>(index,initial={}): 値の初期化、add(v,delta): 点に加算、sum(v,l,r): 距離[l,r)の和。
// ContourAddVertexGet<T>(index,initial={}): 値の初期化、add_range(v,l,r,delta): 距離[l,r)に加算、get(v): 点の値。
// 初期値を省略すると全頂点0。構築は両型O(1+n log(n+1))。以後は各操作O(log²(n+1))。
// 頂点は[0,n)、空木可。負の距離境界・空区間可。距離INT64_MAX以上は半開のint64_t区間には含まれない。
// Tは加法が可換で、T{}は単位元。+=と-=が使えること。加減算は非送出とし、途中の値もTで表現可能なこと。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_CONTOUR_DEFINED
#define SIRKIT_TREE_CONTOUR_DEFINED

struct TreeContourEdge
{
    int from;
    int to;
    std::int64_t length;
};

namespace detail
{
using ContourDistance = std::int64_t;

struct ContourStep
{
    ContourDistance distance;
    int all;
    int branch;
    int all_position;
    int branch_position;
};

struct ContourBucket
{
    std::vector<ContourDistance> distances;
    std::size_t size = 0, offset = 0;
    ContourDistance first = 0;

    void finish()
    {
        std::sort(distances.begin(), distances.end());
        distances.erase(std::unique(distances.begin(), distances.end()), distances.end());
        size = distances.size();
        first = distances.front();
        if (static_cast<std::uint64_t>(distances.back() - first) == size - 1)
        {
            // 無重み木の距離は常に連続するので、検索用の距離表を持たず、添字を直接求める。
            std::vector<ContourDistance>().swap(distances);
        }
    }

    std::size_t lower(ContourDistance distance) const
    {
        if (!distances.empty())
        {
            return static_cast<std::size_t>(std::lower_bound(distances.begin(), distances.end(), distance) - distances.begin());
        }
        if (distance <= first)
        {
            return 0;
        }
        const auto difference = static_cast<std::uint64_t>(distance - first);
        return difference < size ? static_cast<std::size_t>(difference) : size;
    }
};

struct ContourLayout
{
    std::vector<std::vector<ContourStep>> paths;
    std::vector<ContourBucket> buckets;
    std::size_t cells = 0;

    template <class Visit>
    void ranges(int v, ContourDistance left, ContourDistance right, Visit visit) const
    {
        if (left >= right || right <= 0)
        {
            return;
        }
        const auto apply = [&](const ContourBucket &bucket, ContourDistance a, ContourDistance b, bool branch)
        {
            const auto first = bucket.lower(a), last = bucket.lower(b);
            if (first != last)
            {
                visit(bucket, first, last, branch);
            }
        };
        for (const auto &step : paths[v])
        {
            if (step.distance >= right)
            {
                continue;
            }
            const auto a = left > step.distance ? left - step.distance : 0;
            const auto b = right - step.distance;
            // 重心へ向かう同じ枝に属する頂点を除くと、距離の和は実際の経路長になる。
            apply(buckets[step.all], a, b, false);
            if (step.branch >= 0)
            {
                apply(buckets[step.branch], a, b, true);
            }
        }
    }
};

struct ContourArc
{
    int to;
    ContourDistance length;
};

using ContourGraph = std::vector<std::vector<ContourArc>>;

inline TreeContourEdge contour_edge(const std::pair<int, int> &edge)
{
    return {edge.first, edge.second, 1};
}

inline TreeContourEdge contour_edge(TreeContourEdge edge)
{
    return edge;
}

inline ContourDistance contour_distance_add(ContourDistance a, ContourDistance b)
{
    constexpr auto kMaximum = std::numeric_limits<ContourDistance>::max();
    return a > kMaximum - b ? kMaximum : a + b;
}

#ifndef SKIP_RUNTIME_CHECKS
inline void check_contour_tree(const ContourGraph &graph)
{
    std::vector<bool> seen(graph.size());
    std::vector<int> reached{0};
    reached.reserve(graph.size());
    seen[0] = true;
    for (std::size_t i = 0; i < reached.size(); ++i)
    {
        for (const auto &[to, length] : graph[reached[i]])
        {
            if (!seen[to])
            {
                seen[to] = true;
                reached.push_back(to);
            }
        }
    }
    if (reached.size() != graph.size())
    {
        throw std::invalid_argument("TreeContourIndex: graph is not a tree");
    }
}
#endif

class ContourLayoutBuilder
{
    struct Member
    {
        int vertex;
        int parent;
        ContourDistance distance;
    };

    const ContourGraph &graph_;
    ContourLayout &layout_;
    std::vector<bool> removed_;
    std::vector<int> parents_, sizes_, vertices_, pending_{0};
    std::vector<Member> branch_;

    int find_centroid(int root)
    {
        vertices_.clear();
        vertices_.push_back(root);
        parents_[root] = -1;
        for (std::size_t i = 0; i < vertices_.size(); ++i)
        {
            const int v = vertices_[i];
            sizes_[v] = 1;
            for (const auto &[to, length] : graph_[v])
            {
                if (to != parents_[v] && !removed_[to])
                {
                    parents_[to] = v;
                    vertices_.push_back(to);
                }
            }
        }
        const auto half = static_cast<int>(vertices_.size() / 2);
        for (std::size_t i = vertices_.size(); i > 1; --i)
        {
            const int v = vertices_[i - 1];
            // 最初にサイズが半分を超える部分木なら、その子も外側も半分以下。
            if (sizes_[v] > half)
            {
                return v;
            }
            sizes_[parents_[v]] += sizes_[v];
        }
        return root;
    }

    void process_branch(int centroid, int all, ContourArc first)
    {
        const auto id = static_cast<int>(layout_.buckets.size());
        auto &bucket = layout_.buckets.emplace_back();
        auto &whole = layout_.buckets[all];
        branch_.clear();
        branch_.push_back({first.to, centroid, first.length});
        for (std::size_t i = 0; i < branch_.size(); ++i)
        {
            const auto [v, parent, distance] = branch_[i];
            bucket.distances.push_back(distance);
            whole.distances.push_back(distance);
            for (const auto &[to, length] : graph_[v])
            {
                if (to != parent && !removed_[to])
                {
                    branch_.push_back({to, v, contour_distance_add(distance, length)});
                }
            }
        }
        bucket.finish();
        for (const auto &[v, parent, distance] : branch_)
        {
            layout_.paths[v].push_back({distance, all, id, 0, static_cast<int>(bucket.lower(distance))});
        }
    }

public:
    ContourLayoutBuilder(const ContourGraph &graph, ContourLayout &layout)
        : graph_(graph), layout_(layout), removed_(graph.size()), parents_(graph.size()), sizes_(graph.size())
    {
        vertices_.reserve(graph.size());
        branch_.reserve(graph.size());
    }

    void run()
    {
        // 全体の表は頂点数、枝の表は重心分解木の辺数だけ作る。
        layout_.buckets.reserve(2 * graph_.size() - 1);
        while (!pending_.empty())
        {
            const int root = pending_.back();
            pending_.pop_back();
            const int centroid = find_centroid(root);
            removed_[centroid] = true;

            const auto all = static_cast<int>(layout_.buckets.size());
            auto &whole = layout_.buckets.emplace_back();
            whole.distances.reserve(vertices_.size());
            whole.distances.push_back(0);
            layout_.paths[centroid].push_back({0, all, -1, 0, 0});
            for (const auto &arc : graph_[centroid])
            {
                if (!removed_[arc.to])
                {
                    process_branch(centroid, all, arc);
                    pending_.push_back(arc.to);
                }
            }
            whole.finish();
            for (int v : vertices_)
            {
                auto &step = layout_.paths[v].back();
                step.all_position = static_cast<int>(whole.lower(step.distance));
            }
        }

        for (auto &bucket : layout_.buckets)
        {
            bucket.offset = layout_.cells;
            layout_.cells += bucket.size;
        }
    }
};

inline void append_contour_edge(ContourGraph &graph, TreeContourEdge edge, [[maybe_unused]] int n)
{
    const auto [u, v, length] = edge;
#ifndef SKIP_RUNTIME_CHECKS
    if (u < 0 || u >= n || v < 0 || v >= n)
    {
        throw std::out_of_range("TreeContourIndex: vertex out of range");
    }
    if (length < 0)
    {
        throw std::invalid_argument("TreeContourIndex: negative edge length");
    }
#endif
    graph[u].push_back({v, length});
    graph[v].push_back({u, length});
}

template <class Edge>
std::shared_ptr<const ContourLayout> build_contour_layout(int n, std::span<const Edge> edges)
{
    constexpr int kMaxVertices = std::numeric_limits<int>::max() / 2 + 1;
    if (n < 0 || n > kMaxVertices)
    {
        throw std::length_error("TreeContourIndex: vertex count exceeds capacity");
    }
    if (edges.size() != static_cast<std::size_t>(std::max(0, n - 1)))
    {
        throw std::invalid_argument("TreeContourIndex: incorrect number of edges");
    }

    auto result = std::make_shared<ContourLayout>();
    if (n == 0)
    {
        return result;
    }
    ContourGraph graph(n);
    for (const auto &input : edges)
    {
        append_contour_edge(graph, contour_edge(input), n);
    }
#ifndef SKIP_RUNTIME_CHECKS
    check_contour_tree(graph);
#endif
    result->paths.resize(n);
    ContourLayoutBuilder(graph, *result).run();
    return result;
}

inline void check_contour_values(std::size_t supplied, int n)
{
    if (supplied != 0 && supplied != static_cast<std::size_t>(n))
    {
        throw std::invalid_argument("tree contour: incorrect number of initial values");
    }
}

template <class T>
class ContourFenwick
{
    std::vector<T> values_;

public:
    explicit ContourFenwick(std::size_t size) : values_(size)
    {
    }

    void append_initial(const ContourBucket &bucket, std::size_t pos, const T &value)
    {
        values_[bucket.offset + pos] += value;
    }

    void build(const ContourLayout &layout)
    {
        for (const auto &bucket : layout.buckets)
        {
            const auto size = bucket.size;
            for (std::size_t i = 0; i < size; ++i)
            {
                const auto parent = i | (i + 1);
                if (parent < size)
                {
                    values_[bucket.offset + parent] += values_[bucket.offset + i];
                }
            }
        }
    }

    template <bool Subtract = false>
    void add(const ContourBucket &bucket, std::size_t pos, const T &value)
    {
        for (; pos < bucket.size; pos |= pos + 1)
        {
            if constexpr (Subtract)
            {
                values_[bucket.offset + pos] -= value;
            }
            else
            {
                values_[bucket.offset + pos] += value;
            }
        }
    }

    T prefix(const ContourBucket &bucket, std::size_t count) const
    {
        T result{};
        for (; count != 0; count &= count - 1)
        {
            result += values_[bucket.offset + count - 1];
        }
        return result;
    }
};
} // namespace detail

class TreeContourIndex
{
    std::shared_ptr<const detail::ContourLayout> layout_;

    explicit TreeContourIndex(std::shared_ptr<const detail::ContourLayout> layout) : layout_(std::move(layout))
    {
    }

    template <class T>
    friend class VertexAddContourSum;
    template <class T>
    friend class ContourAddVertexGet;

    std::size_t cells() const noexcept
    {
        return layout_ ? layout_->cells : 0;
    }

    void check_vertex([[maybe_unused]] int v) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || v >= size())
        {
            throw std::out_of_range("tree contour: vertex out of range");
        }
#endif
    }

public:
    TreeContourIndex(int n, std::span<const std::pair<int, int>> edges)
        : TreeContourIndex(detail::build_contour_layout(n, edges))
    {
    }

    static TreeContourIndex weighted(int n, std::span<const TreeContourEdge> edges)
    {
        return TreeContourIndex(detail::build_contour_layout(n, edges));
    }

    int size() const noexcept
    {
        return layout_ ? static_cast<int>(layout_->paths.size()) : 0;
    }
};

template <class T = std::int64_t>
class VertexAddContourSum
{
    TreeContourIndex index_;
    detail::ContourFenwick<T> tree_;

public:
    explicit VertexAddContourSum(TreeContourIndex index, std::span<const T> initial = {})
        : index_(std::move(index)), tree_(index_.cells())
    {
        detail::check_contour_values(initial.size(), index_.size());
        if (!initial.empty())
        {
            const auto &layout = *index_.layout_;
            for (std::size_t v = 0; v < initial.size(); ++v)
            {
                for (const auto &step : layout.paths[v])
                {
                    tree_.append_initial(layout.buckets[step.all], step.all_position, initial[v]);
                    if (step.branch >= 0)
                    {
                        tree_.append_initial(layout.buckets[step.branch], step.branch_position, initial[v]);
                    }
                }
            }
            tree_.build(layout);
        }
    }

    void add(int v, const T &delta)
    {
        index_.check_vertex(v);
        const auto &layout = *index_.layout_;
        for (const auto &step : layout.paths[v])
        {
            tree_.add(layout.buckets[step.all], step.all_position, delta);
            if (step.branch >= 0)
            {
                tree_.add(layout.buckets[step.branch], step.branch_position, delta);
            }
        }
    }

    T sum(int v, std::int64_t left, std::int64_t right) const
    {
        index_.check_vertex(v);
        T result{};
        index_.layout_->ranges(v, left, right, [&](const auto &bucket, std::size_t a, std::size_t b, bool branch)
                               {
                                   auto value = tree_.prefix(bucket, b);
                                   value -= tree_.prefix(bucket, a);
                                   if (branch)
                                   {
                                       result -= value;
                                   }
                                   else
                                   {
                                       result += value;
                                   }
                               });
        return result;
    }
};

template <class T = std::int64_t>
class ContourAddVertexGet
{
    TreeContourIndex index_;
    detail::ContourFenwick<T> tree_;
    std::vector<T> initial_;

public:
    explicit ContourAddVertexGet(TreeContourIndex index, std::span<const T> initial = {})
        : index_(std::move(index)), tree_(index_.cells()), initial_(index_.size())
    {
        detail::check_contour_values(initial.size(), index_.size());
        std::copy(initial.begin(), initial.end(), initial_.begin());
    }

    void add_range(int v, std::int64_t left, std::int64_t right, const T &delta)
    {
        index_.check_vertex(v);
        index_.layout_->ranges(v, left, right, [&](const auto &bucket, std::size_t a, std::size_t b, bool)
                               {
                                   tree_.add(bucket, a, delta);
                                   tree_.template add<true>(bucket, b, delta);
                               });
    }

    T get(int v) const
    {
        index_.check_vertex(v);
        auto result = initial_[v];
        const auto &layout = *index_.layout_;
        for (const auto &step : layout.paths[v])
        {
            result += tree_.prefix(layout.buckets[step.all], step.all_position + 1);
            if (step.branch >= 0)
            {
                result -= tree_.prefix(layout.buckets[step.branch], step.branch_position + 1);
            }
        }
        return result;
    }
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
