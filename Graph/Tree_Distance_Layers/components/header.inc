// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_distance_layers.md
//
// API一覧
// TreeDistanceLayers(n,edges): 0始まりの無向森を作り、成分ごとの直径を前計算。
//   edgesは端点の組。距離は辺数。構築・領域はO(n+B log(B+1))、B=ceil(n/32)。
// size(): 頂点数。O(1)。
// farthest(v)/eccentricity(v): 同じ木の最遠頂点の1つとその距離。O(1)。
// diameter_endpoints(v): vが属する木の直径の両端。O(1)。
// vertex_at_distance(v,k): 距離kの頂点の1つ。存在しなければ-1。O(log(n+1))。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
