// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Parity_Orientation.cpp
// 参考（木の辺を決める順序）: https://img.atcoder.jp/agc035/editorial.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/parity_orientation.md
//
// API一覧
// orient_with_outdegree_parity(n,edges,required={}): 無向辺を指定した出次数の偶奇になるように向き付ける。
//   edgesは0始まりの端点の組、required[v]は偶数なら0、奇数なら1。空なら全頂点が偶数。
//   辺の入力順に(from,to)を返す。条件を満たせなければnullopt。O(n+m)時間・領域。
//   非連結、自己ループ、多重辺も可。自己ループは出次数に1を加える。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PARITY_ORIENTATION_DEFINED
#define SIRKIT_PARITY_ORIENTATION_DEFINED

inline std::optional<std::vector<std::pair<int, int>>> orient_with_outdegree_parity(
    int n, std::span<const std::pair<int, int>> edges, std::span<const std::uint8_t> required = {})
{
    std::vector<std::uint8_t> need(n);
    if (!required.empty())
    {
        std::ranges::copy(required, need.begin());
    }

    std::vector<std::vector<std::size_t>> adjacent(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v] = edges[id];
        adjacent[u].push_back(id);
        if (u != v)
        {
            adjacent[v].push_back(id);
        }
    }

    std::vector<std::pair<int, int>> oriented(edges.begin(), edges.end());
    std::vector<bool> entered(n), used(edges.size());
    std::vector<std::size_t> parent_edge(n);
    std::vector<int> order;
    order.reserve(n);
    for (int root = 0; root < n; ++root)
    {
        if (entered[root])
        {
            continue;
        }
        const auto begin = order.size();
        order.push_back(root);
        entered[root] = true;
        for (auto i = begin; i < order.size(); ++i)
        {
            const int u = order[i];
            for (const auto id : adjacent[u])
            {
                if (used[id])
                {
                    continue;
                }
                used[id] = true;
                const auto [a, b] = edges[id];
                const int v = a == u ? b : a;
                if (!entered[v])
                {
                    entered[v] = true;
                    parent_edge[v] = id;
                    order.push_back(v);
                }
                else
                {
                    need[a] ^= 1;
                }
            }
        }

        // 根以外は、親への辺の向きで残りの偶奇を合わせられる。
        for (auto i = order.size(); i > begin + 1;)
        {
            const int u = order[--i];
            auto &[from, to] = oriented[parent_edge[u]];
            if ((from == u) != (need[u] != 0))
            {
                std::swap(from, to);
            }
            need[from] ^= 1;
        }
        if (need[root] != 0)
        {
            return std::nullopt;
        }
    }
    return oriented;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
