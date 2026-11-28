// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subcubic_tree_biconnectivity.md
//
// API一覧
// augment_subcubic_tree_biconnectivity(weights, edges):
//   各頂点の次数が3以下の木に、追加後にどの1頂点を削除しても連結な辺集合を構成する。
//   n>=3、頂点番号は0始まり。辺(u,v)の加算費用weights[u]+weights[v]の総和を最小化。
//   O(n)時間・領域。頂点重みは非負のuint64_t。
//   各枝の葉は巡回順で連続し、半周先への辺で枝どうしをつなぐ。
//   奇数葉では追加頂点を根とし、反対側が単独葉の枝にならない枝の境界に根を挿す。
// BiconnectivityAugmentation{edges,cost}: 追加する辺と最小費用。費用が表現範囲外ならcostはnullopt。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBCUBIC_TREE_BICONNECTIVITY_DEFINED
#define SIRKIT_SUBCUBIC_TREE_BICONNECTIVITY_DEFINED

struct BiconnectivityAugmentation
{
    std::vector<std::pair<int, int>> edges;
    std::optional<std::uint64_t> cost;
};

namespace detail
{
inline void check_subcubic_tree_size(std::size_t vertices, std::size_t edges)
{
    if (vertices < 3 || vertices > static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
        edges != vertices - 1)
    {
        throw std::length_error("augment_subcubic_tree_biconnectivity: invalid tree size");
    }
}
} // namespace detail

[[nodiscard]] inline BiconnectivityAugmentation augment_subcubic_tree_biconnectivity(
    std::span<const std::uint64_t> weights, std::span<const std::pair<int, int>> edges)
{
    detail::check_subcubic_tree_size(weights.size(), edges.size());
    const int n = static_cast<int>(weights.size());
    std::vector<std::array<int, 3>> neighbors(n);
    std::vector<int> degree(n);
    auto add = [&](int u, int v)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("augment_subcubic_tree_biconnectivity: invalid vertex");
        }
#endif
        if (degree[u] == 3)
        {
            throw std::invalid_argument("augment_subcubic_tree_biconnectivity: degree above three");
        }
        neighbors[u][degree[u]++] = v;
    };
    for (const auto &[u, v] : edges)
    {
        add(u, v);
        add(v, u);
    }

    int leaf_count = 0;
    for (const int d : degree)
    {
        leaf_count += d == 1;
    }
    int extra = -1;
    if (leaf_count % 2 != 0)
    {
        for (int v = 0; v < n; ++v)
        {
            // 三叉の木は中心を失うと、中心に接続した辺を使わず3枝をつなぐ必要がある。
            if (leaf_count == 3 && degree[v] == 3)
            {
                continue;
            }
            if (extra == -1 || weights[v] < weights[extra])
            {
                extra = v;
            }
        }
    }

    const int root = extra == -1 ? 0 : extra;
    std::vector<int> order;
    order.reserve(static_cast<std::size_t>(leaf_count) + 1);
    if (degree[root] == 1)
    {
        order.push_back(root);
    }
    std::array<std::size_t, 4> begin{};
    std::vector<std::pair<int, int>> pending;
#ifndef SKIP_RUNTIME_CHECKS
    std::vector<bool> seen(n);
    seen[root] = true;
    int visited = 1;
#endif
    for (int branch = 0; branch < degree[root]; ++branch)
    {
        begin[branch] = order.size();
        pending.emplace_back(neighbors[root][branch], root);
        while (!pending.empty())
        {
            const auto [v, previous] = pending.back();
            pending.pop_back();
#ifndef SKIP_RUNTIME_CHECKS
            if (seen[v])
            {
                throw std::invalid_argument("augment_subcubic_tree_biconnectivity: cycle");
            }
            seen[v] = true;
            ++visited;
#endif
            if (degree[v] == 1)
            {
                order.push_back(v);
            }
            for (int i = 0; i < degree[v]; ++i)
            {
                const int u = neighbors[v][i];
                if (u != previous)
                {
                    pending.emplace_back(u, v);
                }
            }
        }
    }
    begin[degree[root]] = order.size();
#ifndef SKIP_RUNTIME_CHECKS
    if (visited != n)
    {
        throw std::invalid_argument("augment_subcubic_tree_biconnectivity: disconnected");
    }
#endif
    if (extra != -1)
    {
        std::size_t insertion = 0;
        if (degree[root] != 1)
        {
            for (int corner = 0; corner < degree[root]; ++corner)
            {
                const auto at = begin[corner];
                const auto opposite = (at + order.size() / 2) % order.size();
                bool singleton = false;
                for (int branch = 0; branch < degree[root]; ++branch)
                {
                    if (begin[branch] == opposite && begin[branch + 1] == opposite + 1)
                    {
                        singleton = true;
                        break;
                    }
                }
                if (!singleton)
                {
                    insertion = at;
                }
            }
        }
        order.insert(order.begin() + static_cast<std::ptrdiff_t>(insertion), extra);
    }

    BiconnectivityAugmentation result{{}, 0};
    const auto half = order.size() / 2;
    result.edges.reserve(half);
    for (std::size_t i = 0; i < half; ++i)
    {
        result.edges.emplace_back(order[i], order[i + half]);
    }
    for (const int v : order)
    {
        if (!result.cost)
        {
            break;
        }
        const auto weight = weights[v];
        if (weight > std::numeric_limits<std::uint64_t>::max() - *result.cost)
        {
            result.cost.reset();
        }
        else
        {
            *result.cost += weight;
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
