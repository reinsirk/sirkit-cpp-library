// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Dynamic_Bipartiteness.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_bipartiteness.md
//
// API一覧
// DynamicBipartiteness(n=0): n頂点、辺なしで初期化する。O(1)。
// add_edge(u,v): 無向辺を追加し辺IDを返す。自己ループと多重辺も可。償却O(1)。
// remove_edge(id): 有効な辺IDに対応する1本を削除する。O(1)。
// query_bipartite(): グラフ全体の二部性を照会し、回答添字を返す。O(1)。
// solve(): 照会順のvector<bool>を返す。全操作の記録後に呼ぶ。記録を保つ。
// 頂点数n・追加辺数e・照会数qについて、solveはO(n+q+e log(q+1) log(n+2))時間、
// O(n+q+e log(q+1))追加領域。初期頂点番号は0..n-1。
// size()/edge_count()/query_count(): 頂点数、追加辺数、照会数。各O(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DYNAMIC_BIPARTITENESS_DEFINED
#define SIRKIT_DYNAMIC_BIPARTITENESS_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void CheckDynamicBipartitenessLimit(std::size_t count, std::size_t limit)
{
    if (count > limit)
    {
        throw std::length_error("DynamicBipartiteness: too many events");
    }
}
}

class DynamicBipartiteness
{
public:
    explicit DynamicBipartiteness(int n = 0) : vertices_(n)
    {
    }

    std::size_t add_edge(int u, int v)
    {
        const auto id = edges_.size();
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckDynamicBipartitenessLimit(id + 1, std::numeric_limits<EdgeIndex>::max());
        edges_.push_back({u, v, queries_, kNoEnd});
        return id;
    }

    void remove_edge(std::size_t id)
    {
        edges_[id].end = queries_;
    }

    std::size_t query_bipartite()
    {
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckDynamicBipartitenessLimit(queries_ + 1, kNoEnd / 4 + 1);
        return queries_++;
    }

    [[nodiscard]] int size() const noexcept
    {
        return vertices_;
    }

    [[nodiscard]] std::size_t edge_count() const noexcept
    {
        return edges_.size();
    }

    [[nodiscard]] std::size_t query_count() const noexcept
    {
        return queries_;
    }

    [[nodiscard]] std::vector<bool> solve() const
    {
        if (queries_ == 0 || edges_.empty())
        {
            return std::vector<bool>(queries_, true);
        }

        std::vector<std::size_t> offsets;
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckDynamicBipartitenessLimit(queries_, std::bit_floor((offsets.max_size() - 1) / 2));
        const auto base = std::bit_ceil(queries_);
        offsets.resize(2 * base + 1);

        std::vector<EdgeIndex> slots;
        const auto nodes_per_edge = 2 * std::bit_width(base);
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckDynamicBipartitenessLimit(edges_.size(), slots.max_size() / nodes_per_edge);
        const auto visit_interval = [&](const Edge &edge, auto &&visit)
        {
            const auto end = std::min(edge.end, queries_);
            if (edge.begin == end)
            {
                return;
            }

            // 最後の照会まで有効な辺は、照会のない右端まで延長できる。
            auto left = edge.begin + base;
            auto right = (end == queries_ ? base : end) + base;
            for (; left < right; left >>= 1, right >>= 1)
            {
                if (left & 1)
                {
                    visit(left++);
                }
                if (right & 1)
                {
                    visit(--right);
                }
            }
        };

        for (const auto &edge : edges_)
        {
            visit_interval(edge, [&](std::size_t node)
                           {
                               ++offsets[node];
                           });
        }
        std::partial_sum(offsets.begin(), offsets.end(), offsets.begin());
        if (offsets.back() == 0)
        {
            return std::vector<bool>(queries_, true);
        }
        slots.resize(offsets.back());
        for (std::size_t id = 0; id < edges_.size(); ++id)
        {
            visit_interval(edges_[id], [&](std::size_t node)
                           {
                               slots[--offsets[node]] = static_cast<EdgeIndex>(id);
                           });
        }

        std::vector<bool> answers(base);
        ParityState state(vertices_);
        const auto visit_tree = [&](auto &&self, std::size_t node) -> void
        {
            const auto saved = state.snapshot();
            for (auto position = offsets[node]; position < offsets[node + 1]; ++position)
            {
                const auto &edge = edges_[slots[position]];
                if (!state.add(edge.u, edge.v))
                {
                    // ここで矛盾するなら、この部分木に属するすべての照会の答えはfalse。
                    state.rollback(saved);
                    return;
                }
            }
            if (node >= base)
            {
                answers[node - base] = true;
            }
            else
            {
                self(self, 2 * node);
                self(self, 2 * node + 1);
            }
            state.rollback(saved);
        };
        visit_tree(visit_tree, 1);
        answers.resize(queries_);
        return answers;
    }

private:
    using EdgeIndex = std::uint32_t;
    static constexpr auto kNoEnd = std::numeric_limits<std::size_t>::max();

    struct Edge
    {
        int u, v;
        std::size_t begin, end;
    };

    class ParityState
    {
    public:
        explicit ParityState(int n) : parent_(n, -1), parity_(n)
        {
        }

        std::size_t snapshot() const noexcept
        {
            return history_.size();
        }

        bool add(int u, int v)
        {
            auto [root_u, parity_u] = Root(u);
            auto [root_v, parity_v] = Root(v);
            if (root_u == root_v)
            {
                return parity_u != parity_v;
            }
            if (parent_[root_u] > parent_[root_v])
            {
                std::swap(root_u, root_v);
            }
            history_.push_back({root_u, root_v, parent_[root_v]});
            parent_[root_u] += parent_[root_v];
            parent_[root_v] = root_u;
            parity_[root_v] = static_cast<std::uint8_t>(parity_u == parity_v);
            return true;
        }

        void rollback(std::size_t saved) noexcept
        {
            while (history_.size() > saved)
            {
                const auto [root, child, child_size] = history_.back();
                history_.pop_back();
                parent_[root] -= child_size;
                parent_[child] = child_size;
                parity_[child] = 0;
            }
        }

    private:
        struct Change
        {
            int root, child, child_size;
        };

        std::vector<int> parent_;
        std::vector<std::uint8_t> parity_;
        std::vector<Change> history_;

        std::pair<int, bool> Root(int v) const noexcept
        {
            bool parity = false;
            while (parent_[v] >= 0)
            {
                parity = parity != static_cast<bool>(parity_[v]);
                v = parent_[v];
            }
            return {v, parity};
        }
    };

    int vertices_;
    std::size_t queries_ = 0;
    std::vector<Edge> edges_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
