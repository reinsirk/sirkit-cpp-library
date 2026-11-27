// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Complement_Components.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/complement_components.md
//
// API一覧
// complement_components(n,edges): 無向グラフの補グラフの連結成分を頂点列の配列で返す。
// 頂点番号は[0,n)。自己辺・平行辺は結果に影響しない。成分間と成分内の順序は任意。
// 補辺を列挙せずO(n+m)時間・領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COMPLEMENT_COMPONENTS_DEFINED
#define SIRKIT_COMPLEMENT_COMPONENTS_DEFINED

inline std::vector<std::vector<int>> complement_components(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("complement_components: negative vertex count");
    }
    std::vector<std::vector<int>> adjacency(n);
    for (const auto &[u, v] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("complement_components: vertex out of range");
        }
#endif
        adjacency[u].push_back(v);
        adjacency[v].push_back(u);
    }
    std::vector<int> next(n), neighbor_of(n, -1);
    std::iota(next.begin(), next.end(), 1);
    std::vector<std::vector<int>> result;
    int head = 0;
    while (head != n)
    {
        auto &component = result.emplace_back(1, head);
        head = next[head];
        for (std::size_t cursor = 0; cursor < component.size(); ++cursor)
        {
            const int u = component[cursor];
            for (int v : adjacency[u])
            {
                neighbor_of[v] = u;
            }
            int *link = &head;
            while (*link != n)
            {
                const int v = *link;
                if (neighbor_of[v] == u)
                {
                    // スキップした回数は元グラフの辺の本数で抑えられる。
                    link = &next[v];
                }
                else
                {
                    *link = next[v];
                    component.push_back(v);
                }
            }
        }
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
