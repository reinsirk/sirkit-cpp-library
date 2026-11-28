// SPDX-License-Identifier: CC0-1.0
// 参考（Yen 法）: https://doi.org/10.1287/mnsc.17.11.712
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/k_shortest_simple_paths.md
//
// API一覧
// k_shortest_simple_paths(n,edges,source,target,k,undirected=false): 単純路を長さの小さい順に最大 k 本返す。
//   ShortestPathEdge{from,to,weight} は0始まりの端点と非負の uint64_t 重み。自己辺・多重辺可。
//   SimpleWeightedPath{distance,vertices,edges} は距離、頂点列、入力順の辺 ID 列。
//   同じ長さでも辺 ID 列の異なる単純路は別に数える。同距離での順は未規定。
//   始点と終点が同じなら長さ0の路だけ。距離が uint64_t を超える路は除外する。
//   最悪 O(kn(n+m log(m+1)+n log(k+1)+k)) 時間、O(n+m+kn) 領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <optional>
#include <queue>
#include <set>
#include <span>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
