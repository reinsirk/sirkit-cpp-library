// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Graph/Zero_One_BFS.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/zero_one_bfs.md
//
// API一覧
// zero_one_bfs(n,source,neighbors): 0/1の重みを持つグラフで始点からの最短路を求める。
// neighbors(v,visit)は各有向辺にvisit(to,weight)を呼ぶ。呼び出し中に処理を完結する。
// ZeroOnePaths{distance,parent}: 最短距離と前の頂点。到達不能は双方-1。始点の親は始点自身。
// 頂点番号は[0,n)。1頂点の辺列を次数に比例する時間で得られるときO(n+m)時間・領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <deque>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ZERO_ONE_BFS_DEFINED
#define SIRKIT_ZERO_ONE_BFS_DEFINED

struct ZeroOnePaths
{
    std::vector<int> distance;
    std::vector<int> parent;
};

template <class Neighbors>
ZeroOnePaths zero_one_bfs(int n, int source, Neighbors &&neighbors)
{
    if (n < 0)
    {
        throw std::length_error("zero_one_bfs: negative vertex count");
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (source < 0 || source >= n)
    {
        throw std::out_of_range("zero_one_bfs: source out of range");
    }
#endif
    ZeroOnePaths result{std::vector<int>(n, -1), std::vector<int>(n, -1)};
    result.distance[source] = 0;
    result.parent[source] = source;
    std::deque<std::pair<int, int>> queue{{source, 0}};
    while (!queue.empty())
    {
        const auto [vertex, distance] = queue.front();
        queue.pop_front();
        if (result.distance[vertex] != distance)
        {
            continue;
        }
        neighbors(vertex, [&](int to, int weight)
                  {
#ifndef SKIP_RUNTIME_CHECKS
                      if (to < 0 || to >= n)
                      {
                          throw std::out_of_range("zero_one_bfs: vertex out of range");
                      }
                      if (weight != 0 && weight != 1)
                      {
                          throw std::invalid_argument("zero_one_bfs: weight is not 0 or 1");
                      }
#endif
                      const auto next = distance + weight;
                      if (result.distance[to] == -1 || next < result.distance[to])
                      {
                          result.distance[to] = next;
                          result.parent[to] = vertex;
                          if (weight == 0)
                          {
                              queue.emplace_front(to, next);
                          }
                          else
                          {
                              queue.emplace_back(to, next);
                          }
                      }
                  });
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
