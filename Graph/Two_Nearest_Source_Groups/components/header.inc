// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/two_nearest_source_groups.md
//
// API一覧
// ShortestPathEdge{from,to,weight}: 重みが非負の uint64_t の辺。
// GroupedShortestPathSource{vertex,group}: 始点の頂点 ID とグループ ID。
// two_nearest_source_groups(n,edges,sources,undirected=false): 各頂点へ近い始点の上位2グループ。
//   各要素は距離順の optional<SourceGroupDistance> の長さ2の配列。
//   SourceGroupDistance{distance,source,group} は距離、始点の頂点 ID、グループ ID。
//   頂点は0始まり。グループ ID は任意の int。距離が同じ候補の順は未規定。
//   未到達・距離が uint64_t を超える候補は除く。始点数 s、x=m+s とする。
//   時間 O(n+x log(x+1))、全辺の重みが1なら O(n+x)。領域 O(n+x)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
