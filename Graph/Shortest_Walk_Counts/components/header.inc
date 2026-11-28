// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/shortest_walk_counts.md
//
// API一覧
// ShortestPathEdge{from,to,weight}: 0始まりの両端点と非負のuint64_t重み。
// count_shortest_walks<Count=uint64_t>(n,edges,source,undirected=false):
//   始点から各頂点への最短歩道を、辺の列が異なれば別々に数える。自己ループ・多重辺可。
//   結果のdistance[v]は最短距離。到達不能またはuint64_tで表せない距離はnullopt。
//   count[v]は有限の場合の個数、infinite[v]は最短歩道が無限にあるか。未到達・無限のcountは0。
//   Countは0,1から構築でき、加算代入を持つ型。既定のuint64_tは2^64で周回。
//   重みが全部1ならO(n+m)時間、それ以外はO((n+m)log(n+m+1))時間。O(n+m)領域。


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
