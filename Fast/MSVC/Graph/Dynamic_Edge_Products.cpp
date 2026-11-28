// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Dynamic_Edge_Products.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_edge_products.md
//
// API一覧
// DynamicEdgeProducts<T>(n,edges,values={}): 使用する候補無向辺と初期頂点値で準備。全辺は無効から開始。
//   頂点は0始まり。候補辺は単純グラフで番号は入力位置。valuesが空なら初期値0。
//   n頂点・m候補辺。準備O(n+m log(m+1))時間・O(n+m)領域。
// set_active(id,enabled): 辺の有効状態を設定し、変更があったときtrueを返す。active(id): 現在の状態。各O(1)。
// add(v,delta): 頂点値に加算。neighbor_sum(v): 有効な隣接辺の相手側の頂点値の和。各O(1+sqrt(m))時間。
// value(v): 現在の頂点値。sum_edge_products(): 有効辺{u,v}ごとの値[u]*値[v]の総和。各O(1)。
//   Tはboolを除く64bit以下の整数。加減乗算は型幅で周回。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DYNAMIC_EDGE_PRODUCTS_DEFINED
#define SIRKIT_DYNAMIC_EDGE_PRODUCTS_DEFINED

template <std::integral T = std::uint64_t>
    requires(sizeof(T) <= sizeof(std::uint64_t) && !std::is_same_v<T, bool>)
class DynamicEdgeProducts
{
public:
    DynamicEdgeProducts(int n, std::span<const std::pair<int, int>> edges, std::span<const T> values = {})
    {
        if (n < 0)
        {
            throw std::length_error("DynamicEdgeProducts: negative vertex count");
        }
        if (!values.empty() && values.size() != static_cast<std::size_t>(n))
        {
            throw std::invalid_argument("DynamicEdgeProducts: inconsistent value count");
        }
        adjacent_.resize(n);
        values_.resize(n);
        cached_.resize(n);
        heavy_.resize(n);
        heavy_neighbors_.resize(n);
        edges_.assign(edges.begin(), edges.end());
        active_.resize(edges.size());
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            values_[i] = static_cast<std::uint64_t>(values[i]);
        }

        for (std::size_t i = 0; i < edges_.size(); ++i)
        {
            auto &[u, v] = edges_[i];
            check_vertex(u);
            check_vertex(v);
            if (u > v)
            {
                std::swap(u, v);
            }
            adjacent_[u].emplace_back(v, i);
            adjacent_[v].emplace_back(u, i);
        }
        const auto threshold = std::size_t{1} << ((std::bit_width(edges.size()) + 1) / 2);
        for (int v = 0; v < n; ++v)
        {
            heavy_[v] = adjacent_[v].size() > threshold;
        }
        for (int v = 0; v < n; ++v)
        {
            for (const auto &[u, id] : adjacent_[v])
            {
                if (heavy_[u])
                {
                    heavy_neighbors_[v].emplace_back(u, id);
                }
            }
        }
    }

    bool set_active(std::size_t id, bool enabled)
    {
        check_edge(id);
        if (active_[id] == enabled)
        {
            return false;
        }
        const auto &[u, v] = edges_[id];
        const auto sign = enabled ? std::uint64_t{1} : std::uint64_t{0} - 1;
        products_ += sign * values_[u] * values_[v];
        if (heavy_[u])
        {
            cached_[u] += sign * values_[v];
        }
        if (heavy_[v])
        {
            cached_[v] += sign * values_[u];
        }
        active_[id] = enabled;
        return true;
    }

    [[nodiscard]] bool active(std::size_t id) const
    {
        check_edge(id);
        return active_[id];
    }

    void add(int v, T delta)
    {
        check_vertex(v);
        const auto amount = static_cast<std::uint64_t>(delta);
        products_ += amount * neighbor_bits(v);
        values_[v] += amount;
        for (const auto &[u, id] : heavy_neighbors_[v])
        {
            if (active_[id])
            {
                cached_[u] += amount;
            }
        }
    }

    [[nodiscard]] T value(int v) const
    {
        check_vertex(v);
        return static_cast<T>(values_[v]);
    }

    [[nodiscard]] T neighbor_sum(int v) const
    {
        check_vertex(v);
        return static_cast<T>(neighbor_bits(v));
    }

    [[nodiscard]] T sum_edge_products() const
    {
        return static_cast<T>(products_);
    }

private:
    [[nodiscard]] std::uint64_t neighbor_bits(int v) const
    {
        if (heavy_[v])
        {
            return cached_[v];
        }
        std::uint64_t result = 0;
        for (const auto &[u, id] : adjacent_[v])
        {
            if (active_[id])
            {
                result += values_[u];
            }
        }
        return result;
    }

    void check_vertex([[maybe_unused]] int v) const
    {
    }

    void check_edge([[maybe_unused]] std::size_t id) const
    {
    }

    using Incidence = std::pair<int, std::size_t>;
    std::vector<std::pair<int, int>> edges_;
    std::vector<std::vector<Incidence>> adjacent_;
    std::vector<std::vector<Incidence>> heavy_neighbors_;
    std::vector<std::uint64_t> values_;
    std::vector<std::uint64_t> cached_;
    std::vector<bool> active_;
    std::vector<bool> heavy_;
    std::uint64_t products_ = 0;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
