// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Enumerate_Triangles.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/enumerate_triangles.md
//
// API一覧
// enumerate_triangles(n,edges,visit): 単純無向グラフの各三角形についてvisit(u,v,w)を1回呼ぶ。
// 頂点番号は[0,n)。自己辺・重複辺は不可。列挙順と引数の順序は任意。
// コールバックを除きO(n+m√m)時間・O(n+m)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ENUMERATE_TRIANGLES_DEFINED
#define SIRKIT_ENUMERATE_TRIANGLES_DEFINED

template <class Visit>
void enumerate_triangles(int n, std::span<const std::pair<int, int>> edges, Visit &&visit)
{
    if (n < 0)
    {
        throw std::length_error("enumerate_triangles: negative vertex count");
    }
    std::vector<std::size_t> degree(n);
    for (const auto &[u, v] : edges)
    {
        ++degree[u];
        ++degree[v];
    }

    // 次数と頂点番号の順に向きを付けると、出次数は O(√m) に収まる。
    std::vector<std::vector<int>> forward(n);
    for (auto [u, v] : edges)
    {
        if (std::pair{degree[v], v} < std::pair{degree[u], u})
        {
            std::swap(u, v);
        }
        forward[u].push_back(v);
    }
    std::vector<int> marked(n, -1);
    for (int u = 0; u < n; ++u)
    {
        for (int v : forward[u])
        {
            marked[v] = u;
        }
        for (int v : forward[u])
        {
            for (int w : forward[v])
            {
                if (marked[w] == u)
                {
                    visit(u, v, w);
                }
            }
        }
    }
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
