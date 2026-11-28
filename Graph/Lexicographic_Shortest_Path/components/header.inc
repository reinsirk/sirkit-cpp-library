// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lexicographic_shortest_path.md
//
// API一覧
// lexicographic_shortest_path(n,edges,source,target,undirected=false): 頂点列が辞書順最小の最短路。
//   ShortestPathEdge{from,to,weight} の端点は0始まり、重みは正の uint64_t。自己辺・多重辺可。
//   LexicographicShortestPath{distance,vertices,edges} の edges は入力順の辺 ID。
//   頂点列も同じなら辺 ID 列が辞書順最小。到達不能・距離の uint64_t 範囲超過は nullopt。
//   時間 O(n+m log(m+1))、領域 O(n+m)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
