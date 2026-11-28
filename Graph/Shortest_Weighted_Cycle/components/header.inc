// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/shortest_weighted_cycle.md
//
// API一覧
// shortest_weighted_cycle(n, edges, directed=true, through=std::nullopt):
//   重みの和が最小の単純閉路を返す。throughを指定するとその頂点を含む閉路だけを調べる。
//   WeightedCycleEdge{from,to,weight=1}。重みは非負のuint64_t。自己辺と多重辺も扱う。
//   閉路がないか、重みの和がuint64_tの上限を超えるとnullopt。
//   1始点: O((n+m)log(n+m+1))時間。全頂点: O(n(n+m)log(n+m+1))時間。領域O(n+m+1)。
// WeightedCycle{weight,cycle}: 重みの和と、巡回順の頂点列・入力の辺番号列。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <numeric>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
