// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Incremental_Weighted_Forest.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/incremental_weighted_forest.md
//
// API一覧
// IncrementalWeightedForest(n): [0,n)の孤立頂点から開始。O(n log(n+1))時間・領域。
// link(u,v,weight): 別々の木なら重みが非負の無向辺を加えtrue。同じ木なら変更せずfalse。
// 小さい木の頂点数をkとしてO(k log(n+1))時間、全追加の合計O(n log^2(n+1))。
// distance(u,v): 辺重みの和。非連結ならnullopt。O(log(n+1))。
// diameter(v): 木の直径をDiameter{first,second,distance}で返す。O(log(n+1))。
// connected(u,v)/component_size(v): 連結か/木の頂点数。各O(log(n+1))。size(): 全頂点数、O(1)。
// 成分内の辺の総重みはstd::uint64_tに収める。コピーは独立、ムーブ構築はO(1)。

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
#ifndef SIRKIT_INCREMENTAL_WEIGHTED_FOREST_DEFINED
#define SIRKIT_INCREMENTAL_WEIGHTED_FOREST_DEFINED

class IncrementalWeightedForest
{
public:
    struct Diameter
    {
        int first;
        int second;
        std::uint64_t distance;
    };

    explicit IncrementalWeightedForest(int n)
        : components_(vertex_count(n), -1), levels_(std::bit_width(static_cast<unsigned int>(n))),
          depth_(size()), ancestor_(size() * levels_), root_distance_(size()), total_weight_(size()),
          diameter_(size()), graph_(size())
    {
        for (int v = 0; v < n; ++v)
        {
            std::fill_n(ancestor_.begin() + static_cast<std::size_t>(v) * levels_, levels_, v);
            diameter_[v] = {v, v, 0};
        }
    }

    IncrementalWeightedForest(const IncrementalWeightedForest &) = default;
    IncrementalWeightedForest(IncrementalWeightedForest &&) noexcept = default;

    IncrementalWeightedForest &operator=(IncrementalWeightedForest other) noexcept
    {
        components_.swap(other.components_);
        std::swap(levels_, other.levels_);
        depth_.swap(other.depth_);
        ancestor_.swap(other.ancestor_);
        root_distance_.swap(other.root_distance_);
        total_weight_.swap(other.total_weight_);
        diameter_.swap(other.diameter_);
        graph_.swap(other.graph_);
        pending_.swap(other.pending_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return components_.size();
    }

    [[nodiscard]] bool connected(int u, int v) const
    {
        check_vertex(u);
        check_vertex(v);
        return leader(u) == leader(v);
    }

    [[nodiscard]] int component_size(int v) const
    {
        check_vertex(v);
        return -components_[leader(v)];
    }

    [[nodiscard]] std::optional<std::uint64_t> distance(int u, int v) const
    {
        if (!connected(u, v))
        {
            return std::nullopt;
        }
        return distance_connected(u, v);
    }

    [[nodiscard]] Diameter diameter(int v) const
    {
        check_vertex(v);
        return diameter_[leader(v)];
    }

    bool link(int u, int v, std::uint64_t weight)
    {
        check_vertex(u);
        check_vertex(v);
        int big = leader(u), small = leader(v);
        if (big == small)
        {
            return false;
        }
        if (components_[big] > components_[small])
        {
            std::swap(big, small);
            std::swap(u, v);
        }
        const auto sum = checked_sum(checked_sum(total_weight_[big], total_weight_[small]), weight);
        const auto [end_u, length_u] = farthest(u, diameter_[big]);
        const auto [end_v, length_v] = farthest(v, diameter_[small]);
        auto longest = diameter_[big];
        if (diameter_[small].distance > longest.distance)
        {
            longest = diameter_[small];
        }
        const auto cross = length_u + weight + length_v;
        if (cross > longest.distance)
        {
            longest = {end_u, end_v, cross};
        }

        // メモリの確保を先に済ませてから、小さい側の登録位置を変更する。
        reserve_edge(u);
        reserve_edge(v);
        pending_.reserve(static_cast<std::size_t>(-components_[small]));
        pending_.push_back({v, u, weight});
        for (std::size_t i = 0; i < pending_.size(); ++i)
        {
            const auto [here, parent, cost] = pending_[i];
            write_parent(here, parent, cost);
            for (const auto &[next, next_cost] : graph_[here])
            {
                if (next != parent)
                {
                    pending_.push_back({next, here, next_cost});
                }
            }
        }
        pending_.clear();

        graph_[u].push_back({v, weight});
        graph_[v].push_back({u, weight});
        components_[big] += components_[small];
        components_[small] = big;
        total_weight_[big] = sum;
        diameter_[big] = longest;
        return true;
    }

private:
    struct Edge
    {
        int to;
        std::uint64_t weight;
    };
    struct Visit
    {
        int vertex;
        int parent;
        std::uint64_t cost;
    };

    static std::size_t vertex_count(int n)
    {
        if (n < 0)
        {
            throw std::length_error("IncrementalWeightedForest: negative vertex count");
        }
        return static_cast<std::size_t>(n);
    }

    static std::uint64_t checked_sum(std::uint64_t a, std::uint64_t b)
    {
        if (b > std::numeric_limits<std::uint64_t>::max() - a)
        {
            throw std::overflow_error("IncrementalWeightedForest: component weight out of range");
        }
        return a + b;
    }

    void check_vertex(int v) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || static_cast<std::size_t>(v) >= size())
        {
            throw std::out_of_range("IncrementalWeightedForest: vertex");
        }
#else
        static_cast<void>(v);
#endif
    }

    int leader(int v) const noexcept
    {
        while (components_[v] >= 0)
        {
            v = components_[v];
        }
        return v;
    }

    int ancestor(int v, unsigned int level) const noexcept
    {
        return ancestor_[static_cast<std::size_t>(v) * levels_ + level];
    }

    int lca(int u, int v) const noexcept
    {
        if (depth_[u] < depth_[v])
        {
            std::swap(u, v);
        }
        auto difference = static_cast<unsigned int>(depth_[u] - depth_[v]);
        for (; difference != 0; difference &= difference - 1)
        {
            u = ancestor(u, static_cast<unsigned int>(std::countr_zero(difference)));
        }
        if (u == v)
        {
            return u;
        }
        for (auto level = levels_; level != 0; )
        {
            --level;
            const int a = ancestor(u, level), b = ancestor(v, level);
            if (a != b)
            {
                u = a;
                v = b;
            }
        }
        return ancestor(u, 0);
    }

    std::uint64_t distance_connected(int u, int v) const noexcept
    {
        const auto root = root_distance_[lca(u, v)];
        return (root_distance_[u] - root) + (root_distance_[v] - root);
    }

    std::pair<int, std::uint64_t> farthest(int v, Diameter diameter) const noexcept
    {
        const auto first = distance_connected(v, diameter.first);
        const auto second = distance_connected(v, diameter.second);
        return first >= second ? std::pair{diameter.first, first} : std::pair{diameter.second, second};
    }

    void reserve_edge(int v)
    {
        auto &list = graph_[v];
        if (list.size() == list.capacity())
        {
            list.reserve(2 * list.size() + 1);
        }
    }

    void write_parent(int v, int parent, std::uint64_t weight) noexcept
    {
        depth_[v] = depth_[parent] + 1;
        root_distance_[v] = root_distance_[parent] + weight;
        auto *row = ancestor_.data() + static_cast<std::size_t>(v) * levels_;
        row[0] = parent;
        for (unsigned int level = 1; level < levels_; ++level)
        {
            row[level] = ancestor(row[level - 1], level - 1);
        }
    }

    std::vector<int> components_;
    unsigned int levels_;
    std::vector<int> depth_, ancestor_;
    std::vector<std::uint64_t> root_distance_, total_weight_;
    std::vector<Diameter> diameter_;
    std::vector<std::vector<Edge>> graph_;
    std::vector<Visit> pending_;
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
