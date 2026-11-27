// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Tree_Decomposition_Width_Two.cpp
// 木分解の定義: https://webspace.science.uu.nl/~bodla101/dyntw2.pdf
// ハッシュの64ビット変換: https://prng.di.unimi.it/splitmix64.c
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_decomposition_width_two.md
//
// API一覧
// tree_decomposition_width_two(n, edges): 0始まりの無向グラフを幅2以下で木分解する。木幅3以上ならnullopt。
// WidthTwoTreeDecomposition: bagsは袋の列、tree_edgesは袋番号（0始まり）による木の辺、widthは得られた分解の幅。
// WidthTwoBag: verticesの先頭size個（1～3個）が袋の頂点。空グラフでは袋も木の辺も空でwidthは0。
// 多重辺と自己辺を許す。非連結でも袋は1本の木にまとめる。平均O(n+m)時間・O(n+m)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_DECOMPOSITION_WIDTH_TWO_DEFINED
#define SIRKIT_TREE_DECOMPOSITION_WIDTH_TWO_DEFINED

struct WidthTwoBag
{
    std::array<int, 3> vertices{};
    int size = 0;
};

struct WidthTwoTreeDecomposition
{
    std::vector<WidthTwoBag> bags;
    std::vector<std::pair<int, int>> tree_edges;
    int width = 0;
};

namespace detail
{
inline constexpr std::size_t kWidthTwoNoArc = std::numeric_limits<std::size_t>::max();

struct WidthTwoArc
{
    std::size_t next;
    int to;
};

struct WidthTwoEdgeHash
{
    std::size_t operator()(std::uint64_t key) const noexcept
    {
        key ^= key >> 30;
        key *= 0xbf58476d1ce4e5b9ULL;
        key ^= key >> 27;
        key *= 0x94d049bb133111ebULL;
        return static_cast<std::size_t>(key ^ (key >> 31));
    }
};

class WidthTwoGraph
{
    std::vector<std::size_t> heads_;
    std::vector<WidthTwoArc> arcs_;
    std::unordered_set<std::uint64_t, WidthTwoEdgeHash> edges_;

    void link(int from, int to)
    {
        const auto next = heads_[from];
        heads_[from] = arcs_.size();
        arcs_.push_back({next, to});
    }

public:
    WidthTwoGraph(int n, std::size_t m) : heads_(n, kWidthTwoNoArc)
    {
        arcs_.reserve(2 * m);
        edges_.reserve(m);
    }

    bool add(int u, int v)
    {
        const auto [first, second] = std::minmax(u, v);
        const auto key = (static_cast<std::uint64_t>(first) << 32) | static_cast<std::uint32_t>(second);
        if (!edges_.insert(key).second)
        {
            return false;
        }
        link(u, v);
        link(v, u);
        return true;
    }

    WidthTwoBag bag(int v, std::span<const int> degree) const
    {
        WidthTwoBag result{{v, 0, 0}, 1};
        for (auto i = heads_[v]; i != kWidthTwoNoArc; i = arcs_[i].next)
        {
            const int to = arcs_[i].to;
            if (degree[to] >= 0)
            {
                result.vertices[result.size++] = to;
            }
        }
        return result;
    }
};
} // namespace detail

inline std::optional<WidthTwoTreeDecomposition> tree_decomposition_width_two(
    int n, std::span<const std::pair<int, int>> edges)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 0)
    {
        throw std::length_error("tree_decomposition_width_two: negative vertex count");
    }
#endif
    detail::WidthTwoGraph graph(n, edges.size());
    std::vector<int> degree(n);
    for (const auto &[u, v] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("tree_decomposition_width_two: edge endpoint out of range");
        }
#endif
        if (u != v && graph.add(u, v))
        {
            ++degree[u];
            ++degree[v];
        }
    }

    std::vector<int> ready;
    ready.reserve(n);
    for (int v = 0; v < n; ++v)
    {
        if (degree[v] <= 2)
        {
            ready.push_back(v);
        }
    }

    WidthTwoTreeDecomposition result;
    result.bags.reserve(n);
    std::vector<int> position(n);
    for (std::size_t i = 0; i < ready.size(); ++i)
    {
        const int v = ready[i];
        const auto bag = graph.bag(v, degree);
        position[v] = static_cast<int>(i);
        degree[v] = -1;
        result.bags.push_back(bag);
        result.width = std::max(result.width, bag.size - 1);

        // 次数2の頂点を消すときに隣接2頂点を結ぶと、新しい補辺がある場合は両端の次数が変わらない。
        if (bag.size == 3 && graph.add(bag.vertices[1], bag.vertices[2]))
        {
            continue;
        }
        for (int j = 1; j < bag.size; ++j)
        {
            const int to = bag.vertices[j];
            if (--degree[to] == 2)
            {
                ready.push_back(to);
            }
        }
    }
    if (ready.size() != static_cast<std::size_t>(n))
    {
        return std::nullopt;
    }

    if (n > 0)
    {
        result.tree_edges.reserve(n - 1);
    }
    for (int i = 0; i < n - 1; ++i)
    {
        const auto &bag = result.bags[i];
        int parent = n - 1;
        // 残りの頂点を最初に消す袋に結ぶ。成分内で最後の袋は木全体の根に結ぶ。
        for (int j = 1; j < bag.size; ++j)
        {
            parent = std::min(parent, position[bag.vertices[j]]);
        }
        result.tree_edges.emplace_back(i, parent);
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
