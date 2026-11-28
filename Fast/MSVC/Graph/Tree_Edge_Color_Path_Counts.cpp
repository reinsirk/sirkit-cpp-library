// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Tree_Edge_Color_Path_Counts.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_edge_color_path_counts.md
//
// API一覧
// tree_edge_color_path_counts(n,edges): 木のパスに現れる辺色が1種類、2種類の頂点対数をそれぞれ返す。
//   TreeColorEdge{from,to,color}。頂点は0始まり、色は任意のint64_t。
// TreeEdgeColorPathCounts{one_color,two_colors}: 異なる2頂点を順序なしで数える。
//   重心を通るパスだけを左右の枝の色集合から数える。O(n log²(n+1))時間、O(n)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNWEIGHTED_GRAPH_DEFINED
#define SIRKIT_UNWEIGHTED_GRAPH_DEFINED
using Graph = std::vector<std::vector<int>>;
#endif

#ifndef SIRKIT_CENTROID_DECOMPOSITION_DEFINED
#define SIRKIT_CENTROID_DECOMPOSITION_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline void CheckCentroidVertexCount(std::size_t vertices) {
    if (vertices > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("CentroidDecomposition vertex count");
}
}
class CentroidDecomposition {
public:
    struct Ancestor {
        int vertex;
        int distance;
        friend bool operator==(const Ancestor&, const Ancestor&) = default;
    };
private:
    int root_ = -1;
    bool stores_distances_ = false;
    std::vector<int> parent_, depth_, component_size_, order_;
    std::vector<std::vector<Ancestor>> ancestors_;

    void check_vertex(int vertex) const {
        static_cast<void>(vertex);
    }
    void check_distances() const {
    }
    void initialize(const Graph& graph, bool store_distances) {
        const auto n = graph.size();
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckCentroidVertexCount(n);
        stores_distances_ = store_distances;
        parent_.assign(n, -1); depth_.resize(n); component_size_.resize(n); order_.reserve(n);
        if (store_distances) ancestors_.resize(n);
        if (n == 0) return;
        std::vector<unsigned char> removed(n);
        std::vector<int> traversal_parent(n, -2), subtree(n), component;
        std::vector<std::pair<int,int>> pending{{0,-1}};
        struct Visit { int vertex, parent, distance; };
        std::vector<Visit> visits;
        component.reserve(n);
        while (!pending.empty()) {
            const auto [start, above] = pending.back(); pending.pop_back();
            component.clear(); component.push_back(start);
            traversal_parent[static_cast<std::size_t>(start)] = -1;
            for (std::size_t i = 0; i < component.size(); ++i) {
                const int vertex = component[i];
                subtree[static_cast<std::size_t>(vertex)] = 1;
                for (const int to : graph[static_cast<std::size_t>(vertex)]) {
                    if (to == traversal_parent[static_cast<std::size_t>(vertex)]) {
                        continue;
                    }
                    if (removed[static_cast<std::size_t>(to)]) continue;
                    traversal_parent[static_cast<std::size_t>(to)] = vertex;
                    component.push_back(to);
                }
            }
            for (auto it = component.rbegin(); it != component.rend(); ++it) {
                const int above_vertex = traversal_parent[static_cast<std::size_t>(*it)];
                if (above_vertex != -1)
                    subtree[static_cast<std::size_t>(above_vertex)] += subtree[static_cast<std::size_t>(*it)];
            }
            const int count = static_cast<int>(component.size());
            int centroid = start;
            for (;;) {
                int next = -1;
                for (const int to : graph[static_cast<std::size_t>(centroid)]) {
                    if (!removed[static_cast<std::size_t>(to)] &&
                        traversal_parent[static_cast<std::size_t>(to)] == centroid &&
                        subtree[static_cast<std::size_t>(to)] > count / 2) { next = to; break; }
                }
                if (next == -1) break;
                centroid = next;
            }
            parent_[static_cast<std::size_t>(centroid)] = above;
            depth_[static_cast<std::size_t>(centroid)] = above == -1 ? 0 : depth_[static_cast<std::size_t>(above)] + 1;
            component_size_[static_cast<std::size_t>(centroid)] = count;
            order_.push_back(centroid);
            if (above == -1) root_ = centroid;
            if (store_distances) {
                visits.clear(); visits.push_back({centroid, -1, 0});
                for (std::size_t i = 0; i < visits.size(); ++i) {
                    const auto [vertex, from, distance] = visits[i];
                    ancestors_[static_cast<std::size_t>(vertex)].push_back({centroid, distance});
                    for (const int to : graph[static_cast<std::size_t>(vertex)])
                        if (to != from && !removed[static_cast<std::size_t>(to)])
                            visits.push_back({to, vertex, distance + 1});
                }
            }
            removed[static_cast<std::size_t>(centroid)] = 1;
            for (const int to : graph[static_cast<std::size_t>(centroid)])
                if (!removed[static_cast<std::size_t>(to)]) pending.emplace_back(to, centroid);
        }
    }
public:
    CentroidDecomposition() = default;
    explicit CentroidDecomposition(const Graph& graph, bool store_distances = true) { initialize(graph, store_distances); }
    CentroidDecomposition(const CentroidDecomposition&) = default;
    CentroidDecomposition& operator=(const CentroidDecomposition& other) {
        if (this != &other) { CentroidDecomposition next(other); swap(next); }
        return *this;
    }
    CentroidDecomposition(CentroidDecomposition&& other) noexcept { swap(other); }
    CentroidDecomposition& operator=(CentroidDecomposition&& other) noexcept {
        CentroidDecomposition next(std::move(other)); swap(next); return *this;
    }
    void swap(CentroidDecomposition& other) noexcept {
        std::swap(root_, other.root_); std::swap(stores_distances_, other.stores_distances_);
        parent_.swap(other.parent_); depth_.swap(other.depth_); component_size_.swap(other.component_size_);
        order_.swap(other.order_); ancestors_.swap(other.ancestors_);
    }
    friend void swap(CentroidDecomposition& a, CentroidDecomposition& b) noexcept { a.swap(b); }
    void build(const Graph& graph, bool store_distances = true) {
        CentroidDecomposition next(graph, store_distances); swap(next);
    }
    void clear() noexcept {
        root_ = -1; stores_distances_ = false;
        parent_.clear(); depth_.clear(); component_size_.clear(); order_.clear(); ancestors_.clear();
    }
    std::size_t size() const noexcept { return parent_.size(); }
    bool empty() const noexcept { return parent_.empty(); }
    int root() const noexcept { return root_; }
    bool stores_distances() const noexcept { return stores_distances_; }
    int parent(int vertex) const { check_vertex(vertex); return parent_[static_cast<std::size_t>(vertex)]; }
    int depth(int vertex) const { check_vertex(vertex); return depth_[static_cast<std::size_t>(vertex)]; }
    int component_size(int vertex) const { check_vertex(vertex); return component_size_[static_cast<std::size_t>(vertex)]; }
    const std::vector<int>& parents() const noexcept { return parent_; }
    const std::vector<int>& depths() const noexcept { return depth_; }
    const std::vector<int>& order() const noexcept { return order_; }
    const std::vector<Ancestor>& ancestors(int vertex) const {
        check_vertex(vertex); check_distances(); return ancestors_[static_cast<std::size_t>(vertex)];
    }
    std::int64_t distance(int u, int v) const {
        const auto& a = ancestors(u); const auto& b = ancestors(v);
        std::size_t i = 0;
        while (i < a.size() && i < b.size() && a[i].vertex == b[i].vertex) ++i;
        return static_cast<std::int64_t>(a[i - 1].distance) + b[i - 1].distance;
    }
    Graph centroid_tree() const {
        Graph result(size());
        for (std::size_t vertex = 0; vertex < size(); ++vertex) {
            const int above = parent_[vertex];
            if (above != -1) {
                result[vertex].push_back(above);
                result[static_cast<std::size_t>(above)].push_back(static_cast<int>(vertex));
            }
        }
        return result;
    }
};
#endif

#ifndef SIRKIT_TREE_EDGE_COLOR_PATH_COUNTS_DEFINED
#define SIRKIT_TREE_EDGE_COLOR_PATH_COUNTS_DEFINED

struct TreeColorEdge
{
    int from;
    int to;
    std::int64_t color;
};

struct TreeEdgeColorPathCounts
{
    std::uint64_t one_color{};
    std::uint64_t two_colors{};
};

[[nodiscard]] inline TreeEdgeColorPathCounts tree_edge_color_path_counts(
    int n, std::span<const TreeColorEdge> edges)
{
    if (n < 0 || edges.size() != static_cast<std::size_t>(n == 0 ? 0 : n - 1))
    {
        throw std::length_error("tree_edge_color_path_counts: inconsistent tree size");
    }

    std::vector<std::int64_t> colors;
    colors.reserve(edges.size());
    for (const auto &edge : edges)
    {
        colors.push_back(edge.color);
    }
    std::ranges::sort(colors);
    colors.erase(std::unique(colors.begin(), colors.end()), colors.end());

    Graph graph(n);
    std::vector<std::vector<std::pair<int, int>>> colored(n);
    for (const auto &[u, v, label] : edges)
    {
        const int color = static_cast<int>(std::lower_bound(colors.begin(), colors.end(), label) - colors.begin());
        graph[u].push_back(v);
        graph[v].push_back(u);
        colored[u].emplace_back(v, color);
        colored[v].emplace_back(u, color);
    }

    const CentroidDecomposition layout(graph, false);
    std::vector<bool> removed(n);
    std::vector<int> one(colors.size()), two_containing(colors.size()), touched;
    std::map<std::pair<int, int>, int> two;
    struct Visit
    {
        int vertex;
        int parent;
        int first;
        int second;
    };
    std::vector<Visit> pending;
    std::vector<std::pair<int, int>> branch;
    TreeEdgeColorPathCounts result;
    for (const int centroid : layout.order())
    {
        removed[centroid] = true;
        int one_total = 0;
        for (const auto &[start, color] : colored[centroid])
        {
            if (removed[start])
            {
                continue;
            }
            branch.clear();
            pending.push_back({start, centroid, color, -1});
            while (!pending.empty())
            {
                const auto [v, previous, first, second] = pending.back();
                pending.pop_back();
                branch.emplace_back(first, second);
                for (const auto &[u, next_color] : colored[v])
                {
                    if (u == previous || removed[u])
                    {
                        continue;
                    }
                    if (next_color == first || next_color == second)
                    {
                        pending.push_back({u, v, first, second});
                    }
                    else if (second == -1)
                    {
                        const auto [low, high] = std::minmax(first, next_color);
                        pending.push_back({u, v, low, high});
                    }
                }
            }

            for (const auto &[first, second] : branch)
            {
                if (second == -1)
                {
                    result.one_color += 1 + one[first];
                    result.two_colors += one_total - one[first] + two_containing[first];
                }
                else
                {
                    result.two_colors += 1 + one[first] + one[second] + two[{first, second}];
                }
            }
            for (const auto &[first, second] : branch)
            {
                touched.push_back(first);
                if (second == -1)
                {
                    ++one_total;
                    ++one[first];
                }
                else
                {
                    touched.push_back(second);
                    ++two_containing[first];
                    ++two_containing[second];
                    ++two[{first, second}];
                }
            }
        }
        for (const int color : touched)
        {
            one[color] = two_containing[color] = 0;
        }
        touched.clear();
        two.clear();
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
