// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Graph/Forest_Coloring.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/forest_coloring.md
//
// API一覧
// ForestColoring(n,edges): 0始まりの無向森。閉路、多重辺、自己辺があれば例外。構築 O(n+m+1) 時間・領域。
// edge_coloring(): 接する辺はすべて異なる色。最小色数は最大次数Δ。
// distance_two_coloring(): 距離が1または2の頂点を異なる色にする。空でない森の最小色数はΔ+1。
//   ForestColoringResult{color_count,colors}。色は0始まり。辺は入力順、頂点は番号順。空なら0色。
// count_distance_two<T>(palette): 使用可能な色をpalette個とした頂点彩色の個数。空の森は1通り。
//   T{0},T{1}とpalette以下の整数から構築でき、乗算可能。各操作 O(n+1) 時間。数え上げの追加領域は O(1)。非再帰。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FOREST_COLORING_DEFINED
#define SIRKIT_FOREST_COLORING_DEFINED

struct ForestColoringResult
{
    int color_count;
    std::vector<int> colors;
};

class ForestColoring
{
public:
    ForestColoring(int n, std::span<const std::pair<int, int>> edges) : edge_count_(edges.size())
    {
        if (n < 0)
        {
            throw std::length_error("ForestColoring: negative vertex count");
        }
        adjacent_.resize(n);
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            const auto [u, v] = edges[i];
            if (u < 0 || u >= n || v < 0 || v >= n)
            {
                throw std::out_of_range("ForestColoring: edge endpoint");
            }
            adjacent_[u].push_back({v, i});
            adjacent_[v].push_back({u, i});
        }

        constexpr int kUnvisited = -2;
        parent_.assign(n, kUnvisited);
        parent_edge_.assign(n, edge_count_);
        order_.reserve(n);
        for (int root = 0; root < n; ++root)
        {
            if (parent_[root] != kUnvisited)
            {
                continue;
            }
            parent_[root] = -1;
            const auto first = order_.size();
            order_.push_back(root);
            for (std::size_t i = first; i < order_.size(); ++i)
            {
                const int v = order_[i];
                for (const auto &[next, edge] : adjacent_[v])
                {
                    if (edge == parent_edge_[v])
                    {
                        continue;
                    }
                    if (parent_[next] != kUnvisited)
                    {
                        throw std::invalid_argument("ForestColoring: graph contains a cycle");
                    }
                    parent_[next] = v;
                    parent_edge_[next] = edge;
                    order_.push_back(next);
                }
                max_degree_ = std::max(max_degree_, static_cast<int>(adjacent_[v].size()));
            }
        }
    }

    [[nodiscard]] ForestColoringResult edge_coloring() const
    {
        if (order_.empty())
        {
            return {0, {}};
        }
        ForestColoringResult result{max_degree_, std::vector<int>(edge_count_)};
        for (const int v : order_)
        {
            const auto upward = parent_edge_[v];
            const int forbidden = upward == edge_count_ ? -1 : result.colors[upward];
            int next_color = 0;
            for (const auto &[child, edge] : adjacent_[v])
            {
                if (edge != upward)
                {
                    if (next_color == forbidden)
                    {
                        ++next_color;
                    }
                    result.colors[edge] = next_color++;
                }
            }
        }
        return result;
    }

    [[nodiscard]] ForestColoringResult distance_two_coloring() const
    {
        if (order_.empty())
        {
            return {0, {}};
        }
        ForestColoringResult result{max_degree_ + 1, std::vector<int>(adjacent_.size())};
        for (const int v : order_)
        {
            const int parent = parent_[v];
            const int parent_color = parent == -1 ? -1 : result.colors[parent];
            int next_color = 0;
            for (const auto &[child, edge] : adjacent_[v])
            {
                if (child != parent)
                {
                    while (next_color == result.colors[v] || next_color == parent_color)
                    {
                        ++next_color;
                    }
                    result.colors[child] = next_color++;
                }
            }
        }
        return result;
    }

    template <class T>
    [[nodiscard]] T count_distance_two(std::uint64_t palette) const
    {
        if (order_.empty())
        {
            return T{1};
        }
        if (palette < static_cast<std::uint64_t>(max_degree_) + 1)
        {
            return T{0};
        }

        T total{1};
        for (const int v : order_)
        {
            const bool root = parent_[v] == -1;
            if (root)
            {
                total = total * T(palette);
            }
            const auto children = adjacent_[v].size() - static_cast<std::size_t>(!root);
            const auto available = palette - (root ? 1U : 2U);
            for (std::size_t position = 0; position < children; ++position)
            {
                total = total * T(available - position);
            }
        }
        return total;
    }

private:
    std::size_t edge_count_;
    int max_degree_ = 0;
    std::vector<std::vector<std::pair<int, std::size_t>>> adjacent_;
    std::vector<int> parent_, order_;
    std::vector<std::size_t> parent_edge_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
