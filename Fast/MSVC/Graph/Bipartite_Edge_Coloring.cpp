// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Bipartite_Edge_Coloring.cpp
// 参考（Noga Alonの二部多重グラフ辺彩色）: https://web.math.princeton.edu/~nalon/PDFS/lex2.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bipartite_edge_coloring.md
//
// API一覧
// bipartite_edge_coloring(left,right,edges): 最大次数と同じ色数で辺を彩色する。
// 辺{u,v}の両端はそれぞれ[0,left)と[0,right)。多重辺を個別に彩色し、同じ頂点を共有する辺は異なる色。
// BipartiteEdgeColoringResult: color_countは色数、colors[i]は入力i番目の辺の色。色は0始まり。
// 辺がなければ色数0、colorsは空。O(left+right+m log(m+1))時間・O(left+right+m)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BIPARTITE_EDGE_COLORING_DEFINED
#define SIRKIT_BIPARTITE_EDGE_COLORING_DEFINED

struct BipartiteEdgeColoringResult
{
    std::size_t color_count;
    std::vector<std::size_t> colors;
};

namespace detail
{
struct EdgeColoringGroups
{
    std::vector<int> vertex_group;
    std::vector<std::size_t> degrees;
};

inline EdgeColoringGroups group_edge_coloring_side(std::span<const std::size_t> degree, std::size_t maximum)
{
    EdgeColoringGroups groups{std::vector<int>(degree.size()), {}};
    for (std::size_t u = 0; u < degree.size(); ++u)
    {
        const auto d = degree[u];
        if (d == 0)
        {
            continue;
        }
        if (groups.degrees.empty() || d > maximum - groups.degrees.back())
        {
            groups.degrees.push_back(d);
        }
        else
        {
            groups.degrees.back() += d;
        }
        groups.vertex_group[u] = static_cast<int>(groups.degrees.size() - 1);
    }
    return groups;
}

struct EdgeColoringEdge
{
    int left;
    int right;
};

class BipartiteEdgeColoringSolver
{
    static constexpr auto kNone = std::numeric_limits<std::size_t>::max();
    static constexpr unsigned char kUnseen = 2;

    std::size_t side_;
    std::vector<EdgeColoringEdge> edges_;
    std::vector<std::size_t> colors_;
    std::vector<std::size_t> heads_, links_;
    std::vector<unsigned char> flags_;

    auto alternating_parts(std::span<const std::size_t> ids) -> std::span<const unsigned char>
    {
        heads_.assign(side_ * 2, kNone);
        links_.resize(ids.size() * 2);
        flags_.assign(ids.size(), kUnseen);
        for (std::size_t i = 0; i < ids.size(); ++i)
        {
            const auto &[left, right] = edges_[ids[i]];
            links_[2 * i] = std::exchange(heads_[left], 2 * i);
            links_[2 * i + 1] = std::exchange(heads_[side_ + right], 2 * i + 1);
        }

        for (std::size_t start = 0; start < side_; ++start)
        {
            auto vertex = start;
            unsigned char part = 0;
            // 偶数次数なら行き止まりは始点になる。二部グラフなので閉歩道の長さも偶数。
            for (;;)
            {
                auto &head = heads_[vertex];
                while (head != kNone && flags_[head / 2] != kUnseen)
                {
                    head = links_[head];
                }
                if (head == kNone)
                {
                    break;
                }
                const auto arc = head;
                head = links_[arc];
                const auto index = arc / 2;
                flags_[index] = part;
                part ^= 1;
                const auto &[left, right] = edges_[ids[index]];
                vertex = (arc & 1) != 0 ? static_cast<std::size_t>(left) : side_ + right;
            }
        }
        return flags_;
    }

    std::array<std::vector<std::size_t>, 2> split(std::span<const std::size_t> ids)
    {
        const auto parts = alternating_parts(ids);
        std::array<std::vector<std::size_t>, 2> bins;
        for (auto &bin : bins)
        {
            bin.reserve(ids.size() / 2);
        }
        for (std::size_t i = 0; i < ids.size(); ++i)
        {
            bins[parts[i]].push_back(ids[i]);
        }
        return bins;
    }

    void color_power_of_two(std::span<const std::size_t> ids, std::size_t degree, std::size_t first_color)
    {
        if (degree == 1)
        {
            for (auto id : ids)
            {
                colors_[id] = first_color;
            }
            return;
        }
        const auto halves = split(ids);
        color_power_of_two(halves[0], degree / 2, first_color);
        color_power_of_two(halves[1], degree / 2, first_color + degree / 2);
    }

    void color_perfect_matching(std::span<const std::size_t> ids, std::size_t degree, std::size_t color)
    {
        struct Multiplicity
        {
            std::size_t id;
            std::size_t copies;
        };

        auto inflated_degree = std::bit_ceil(ids.size());
        const auto factor = inflated_degree / degree;
        const auto diagonal = inflated_degree % degree;
        const auto padding_begin = colors_.size();
        std::vector<Multiplicity> support;
        support.reserve(ids.size() + side_);
        for (auto id : ids)
        {
            support.push_back({id, factor});
        }
        for (std::size_t i = 0; i < side_; ++i)
        {
            support.push_back({padding_begin + i, diagonal});
        }

        // 各回で補助辺の少ない半分を選ぶ。始めに補助辺が次数未満なので最後は0本。
        std::vector<std::size_t> odd_ids, odd_slots;
        while (inflated_degree > 1)
        {
            odd_ids.clear();
            odd_slots.clear();
            for (std::size_t i = 0; i < support.size(); ++i)
            {
                auto &edge = support[i];
                if ((edge.copies & 1) != 0)
                {
                    odd_ids.push_back(edge.id);
                    odd_slots.push_back(i);
                }
                edge.copies /= 2;
            }
            if (!odd_ids.empty())
            {
                const auto parts = alternating_parts(odd_ids);
                std::array<std::size_t, 2> extra{};
                for (std::size_t i = 0; i < odd_ids.size(); ++i)
                {
                    if (odd_ids[i] >= padding_begin)
                    {
                        ++extra[parts[i]];
                    }
                }
                const unsigned char chosen = extra[1] < extra[0] ? 1 : 0;
                for (std::size_t i = 0; i < odd_ids.size(); ++i)
                {
                    if (parts[i] == chosen)
                    {
                        ++support[odd_slots[i]].copies;
                    }
                }
            }
            std::erase_if(support, [](const Multiplicity &edge)
                          {
                              return edge.copies == 0;
                          });
            inflated_degree /= 2;
        }
        for (const auto &edge : support)
        {
            colors_[edge.id] = color;
        }
    }

    void color_regular(std::vector<std::size_t> ids, std::size_t degree, std::size_t first_color)
    {
        if ((degree & 1) != 0)
        {
            const auto color = first_color + degree - 1;
            color_perfect_matching(ids, degree, color);
            std::erase_if(ids, [&](auto id)
                          {
                              return colors_[id] == color;
                          });
            --degree;
        }
        if (std::has_single_bit(degree))
        {
            color_power_of_two(ids, degree, first_color);
            return;
        }

        auto halves = split(ids);
        const auto half = degree / 2;
        color_regular(std::move(halves[0]), half, first_color);
        // 片方の色をいくつか移すと、残りは2冪次数になる。
        const auto power = std::bit_ceil(half);
        const auto begin = first_color + degree - power;
        for (auto id : ids)
        {
            if (colors_[id] >= begin && colors_[id] < first_color + half)
            {
                halves[1].push_back(id);
            }
        }
        color_power_of_two(halves[1], power, begin);
    }

public:
    BipartiteEdgeColoringSolver(std::size_t side, std::vector<EdgeColoringEdge> edges)
        : side_(side), edges_(std::move(edges)), colors_(edges_.size(), kNone)
    {
        for (std::size_t v = 0; v < side_; ++v)
        {
            edges_.push_back({static_cast<int>(v), static_cast<int>(v)});
        }
    }

    std::vector<std::size_t> run(std::size_t degree, std::size_t originals)
    {
        std::vector<std::size_t> ids(colors_.size());
        std::iota(ids.begin(), ids.end(), std::size_t{0});
        color_regular(std::move(ids), degree, 0);
        colors_.resize(originals);
        return std::move(colors_);
    }
};
} // namespace detail

inline BipartiteEdgeColoringResult bipartite_edge_coloring(
    int left, int right, std::span<const std::pair<int, int>> edges)
{
    if (left < 0 || right < 0)
    {
        throw std::length_error("bipartite_edge_coloring: negative vertex count");
    }
    if (edges.empty())
    {
        return {0, {}};
    }
    std::vector<std::size_t> left_degree(left), right_degree(right);
    std::size_t degree = 0;
    for (const auto &[u, v] : edges)
    {
        degree = std::max({degree, ++left_degree[u], ++right_degree[v]});
    }
    if (degree == 1)
    {
        return {1, std::vector<std::size_t>(edges.size())};
    }
    if (degree == edges.size())
    {
        std::vector<std::size_t> colors(edges.size());
        std::iota(colors.begin(), colors.end(), std::size_t{0});
        return {degree, std::move(colors)};
    }

    auto left_groups = detail::group_edge_coloring_side(left_degree, degree);
    auto right_groups = detail::group_edge_coloring_side(right_degree, degree);
    // 連続するグループの次数和はdegreeを超える。側を揃えても正則化後は3m本以下。
    const auto side = std::max(left_groups.degrees.size(), right_groups.degrees.size());
    left_groups.degrees.resize(side);
    right_groups.degrees.resize(side);
    std::vector<detail::EdgeColoringEdge> regular;
    regular.reserve(side * (degree + 1));
    for (const auto &[u, v] : edges)
    {
        regular.push_back({left_groups.vertex_group[u], right_groups.vertex_group[v]});
    }

    std::size_t r = 0;
    for (std::size_t l = 0; l < side; ++l)
    {
        auto missing = degree - left_groups.degrees[l];
        while (missing != 0)
        {
            while (right_groups.degrees[r] == degree)
            {
                ++r;
            }
            const auto count = std::min(missing, degree - right_groups.degrees[r]);
            regular.insert(regular.end(), count, {static_cast<int>(l), static_cast<int>(r)});
            missing -= count;
            right_groups.degrees[r] += count;
        }
    }
    detail::BipartiteEdgeColoringSolver solver(side, std::move(regular));
    return {degree, solver.run(degree, edges.size())};
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
