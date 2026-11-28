// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Undirected_Pseudoforest.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/undirected_pseudoforest.md
//
// API一覧
// UndirectedPseudoforest::build(n, edges): 各連結成分が木か閉路1個の無向グラフを分解。
//   2個以上の閉路を持つ成分があればnullopt。自己辺と多重辺を扱う。O(n+m)時間・領域。
// size()/component_count()/cycle_count(): 頂点数/連結成分数/閉路数。O(1)。
// component(v): vの連結成分を代表する頂点。O(1)。
// root(v)/depth(v): 付属木の根/そこまでの辺数。閉路を持つ成分では閉路上に根を取る。O(1)。
// cycle_id(v): vの連結成分の閉路番号。木なら-1。O(1)。
// cycle(id): 閉路を巡回順の頂点列と辺番号列で返す。O(1)。
// unique_path(u,v): uからvへの単純パスがちょうど1本ならtrue。O(1)。
// distance(u,v): 最短パスの辺数。非連結なら-1。O(log(n+1))。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNDIRECTED_PSEUDOFOREST_DEFINED
#define SIRKIT_UNDIRECTED_PSEUDOFOREST_DEFINED

class UndirectedPseudoforest
{
    struct Arc
    {
        int to;
        int id;
    };

public:
    struct Cycle
    {
        std::span<const int> vertices;
        std::span<const int> edge_ids;
    };

    static std::optional<UndirectedPseudoforest> build(int n, std::span<const std::pair<int, int>> edges)
    {
        if (n < 0)
        {
            throw std::length_error("UndirectedPseudoforest: negative size");
        }
        if (edges.size() > static_cast<std::size_t>(n))
        {
            return std::nullopt;
        }
        UndirectedPseudoforest result(n);
        std::vector<std::vector<Arc>> adjacency(n);
        for (std::size_t id = 0; id < edges.size(); ++id)
        {
            const auto &[u, v] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
            result.check_vertex(u);
            result.check_vertex(v);
#endif
            adjacency[u].push_back({v, static_cast<int>(id)});
            adjacency[v].push_back({u, static_cast<int>(id)});
        }

        std::vector<std::size_t> degree(n);
        std::vector<int> peeled;
        peeled.reserve(n);
        for (int v = 0; v < n; ++v)
        {
            degree[v] = adjacency[v].size();
            if (degree[v] <= 1)
            {
                peeled.push_back(v);
            }
        }
        for (std::size_t i = 0; i < peeled.size(); ++i)
        {
            const int v = peeled[i];
            degree[v] = 0;
            for (const Arc &arc : adjacency[v])
            {
                if (degree[arc.to] != 0)
                {
                    result.parent_[v] = arc.to;
                    if (--degree[arc.to] == 1)
                    {
                        peeled.push_back(arc.to);
                    }
                    break;
                }
            }
        }
        if (std::ranges::any_of(degree, [](std::size_t d)
                                {
                                    return d > 2;
                                }))
        {
            return std::nullopt;
        }
        result.build_cycles(adjacency, degree);
        result.build_attachment_trees(peeled);
        return result;
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(parent_.size());
    }

    [[nodiscard]] int component_count() const noexcept
    {
        return tree_count_ + cycle_count();
    }

    [[nodiscard]] int cycle_count() const noexcept
    {
        return static_cast<int>(cycle_offsets_.size() - 1);
    }

    [[nodiscard]] int component(int v) const
    {
        check_vertex(v);
        return component_[v];
    }

    [[nodiscard]] int root(int v) const
    {
        check_vertex(v);
        return root_[v];
    }

    [[nodiscard]] int depth(int v) const
    {
        check_vertex(v);
        return depth_[v];
    }

    [[nodiscard]] int cycle_id(int v) const
    {
        return cycle_id_[root(v)];
    }

    [[nodiscard]] Cycle cycle(int id) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (id < 0 || id >= cycle_count())
        {
            throw std::out_of_range("UndirectedPseudoforest: invalid cycle");
        }
#endif
        const auto first = cycle_offsets_[id];
        const auto length = cycle_offsets_[id + 1] - first;
        return {std::span(cycle_vertices_).subspan(first, length),
                std::span(cycle_edges_).subspan(first, length)};
    }

    [[nodiscard]] bool unique_path(int u, int v) const
    {
        return root(u) == root(v);
    }

    [[nodiscard]] int distance(int u, int v) const
    {
        if (component(u) != component(v))
        {
            return -1;
        }
        const int a = root_[u];
        const int b = root_[v];
        if (a != b)
        {
            const int length = static_cast<int>(cycle(cycle_id_[a]).vertices.size());
            const int along = std::abs(position_[a] - position_[b]);
            return depth_[u] + depth_[v] + std::min(along, length - along);
        }
        const int lca = attachment_lca(u, v);
        return (depth_[u] - depth_[lca]) + (depth_[v] - depth_[lca]);
    }

private:
    explicit UndirectedPseudoforest(int n)
        : parent_(n, -1), root_(n, -1), depth_(n), head_(n), component_(n),
          cycle_id_(n, -1), position_(n), cycle_offsets_{0}
    {
    }

    void build_cycles(const std::vector<std::vector<Arc>> &adjacency, const std::vector<std::size_t> &degree)
    {
        for (int start = 0; start < size(); ++start)
        {
            if (degree[start] == 0 || root_[start] != -1)
            {
                continue;
            }
            const int id = cycle_count();
            const auto begin = cycle_offsets_.back();
            int vertex = start;
            int last_edge = -1;
            do
            {
                root_[vertex] = vertex;
                head_[vertex] = vertex;
                component_[vertex] = start;
                cycle_id_[vertex] = id;
                position_[vertex] = static_cast<int>(cycle_vertices_.size() - begin);
                cycle_vertices_.push_back(vertex);
                // A core vertex has exactly two incident arcs. Use the other edge to advance.
                const auto arc = std::ranges::find_if(adjacency[vertex], [&](const Arc &e)
                                                      {
                                                          return degree[e.to] != 0 && e.id != last_edge;
                                                      });
                last_edge = arc->id;
                cycle_edges_.push_back(last_edge);
                vertex = arc->to;
            } while (vertex != start);
            cycle_offsets_.push_back(cycle_vertices_.size());
        }
    }

    void build_attachment_trees(const std::vector<int> &peeled)
    {
        std::vector<int> subtree(size(), 1);
        std::vector<int> heavy(size(), -1);
        for (const int v : peeled)
        {
            const int p = parent_[v];
            if (p != -1)
            {
                subtree[p] += subtree[v];
                if (heavy[p] == -1 || subtree[heavy[p]] < subtree[v])
                {
                    heavy[p] = v;
                }
            }
        }
        for (auto it = peeled.rbegin(); it != peeled.rend(); ++it)
        {
            const int v = *it;
            const int p = parent_[v];
            if (p == -1)
            {
                root_[v] = head_[v] = component_[v] = v;
                ++tree_count_;
            }
            else
            {
                root_[v] = root_[p];
                component_[v] = component_[p];
                head_[v] = heavy[p] == v ? head_[p] : v;
                depth_[v] = depth_[p] + 1;
            }
        }
    }

    [[nodiscard]] int attachment_lca(int u, int v) const
    {
        while (head_[u] != head_[v])
        {
            if (depth_[head_[u]] < depth_[head_[v]])
            {
                std::swap(u, v);
            }
            u = parent_[head_[u]];
        }
        return depth_[u] < depth_[v] ? u : v;
    }

    void check_vertex(int v) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || v >= size())
        {
            throw std::out_of_range("UndirectedPseudoforest: invalid vertex");
        }
#else
        static_cast<void>(v);
#endif
    }

    int tree_count_ = 0;
    std::vector<int> parent_;
    std::vector<int> root_;
    std::vector<int> depth_;
    std::vector<int> head_;
    std::vector<int> component_;
    std::vector<int> cycle_id_;
    std::vector<int> position_;
    std::vector<std::size_t> cycle_offsets_;
    std::vector<int> cycle_vertices_;
    std::vector<int> cycle_edges_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
