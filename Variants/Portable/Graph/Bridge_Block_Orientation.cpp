// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Bridge_Block_Orientation.cpp
// 参考（DFSによる向き付け）: https://atcoder.jp/contests/arc111/editorial/492
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bridge_block_orientation.md
//
// API一覧
// orient_bridge_blocks(n,edges): 0始まりの無向グラフの全辺に向きを付ける。入力順の(from,to)を返す。
//   結果の強連結成分が、元のグラフから橋をすべて除いた成分と一致する。
//   非連結、自己ループ、多重辺も可。DFSは再帰を使わない。O(n+m)時間・領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BRIDGE_BLOCK_ORIENTATION_DEFINED
#define SIRKIT_BRIDGE_BLOCK_ORIENTATION_DEFINED

inline std::vector<std::pair<int, int>> orient_bridge_blocks(int n, std::span<const std::pair<int, int>> edges)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 0)
    {
        throw std::invalid_argument("bridge block orientation: negative vertex count");
    }
#endif
    std::vector<std::vector<std::size_t>> adjacent(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("bridge block orientation: endpoint out of range");
        }
#endif
        adjacent[u].push_back(id);
        if (u != v)
        {
            adjacent[v].push_back(id);
        }
    }

    std::vector<std::pair<int, int>> oriented(edges.size(), {-1, -1});
    std::vector<bool> entered(n);
    std::vector<std::pair<int, std::size_t>> active;
    active.reserve(n);
    for (int root = 0; root < n; ++root)
    {
        if (entered[root])
        {
            continue;
        }
        entered[root] = true;
        active.emplace_back(root, 0);
        while (!active.empty())
        {
            auto &[u, cursor] = active.back();
            if (cursor == adjacent[u].size())
            {
                active.pop_back();
                continue;
            }
            const auto id = adjacent[u][cursor++];
            if (oriented[id].first != -1)
            {
                continue;
            }
            const auto [a, b] = edges[id];
            const int v = a == u ? b : a;
            // 初めて調べる非木辺は、自己ループかDFSの祖先へ進む辺。
            oriented[id] = {u, v};
            if (!entered[v])
            {
                entered[v] = true;
                active.emplace_back(v, 0);
            }
        }
    }
    return oriented;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
