// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_tree_distances.md
//
// API一覧
// DynamicTreeDistanceEdge<T>{from,to,weight}: 0始まりの無向辺。入力順が辺ID。
// DynamicTreeDistances<T=int64_t>(n,edges,root=0): 辺重み付き無向森を作る。形は固定。
//   構築・領域O(n+B log(B+1))、B=ceil(n/32)。別成分は最小番号の頂点を根とする。
// size()/edge_count()/weight(id): 頂点数、辺数、辺重み。O(1)。
// set_weight(id,w): 辺の重みを上書き。O(log(n+1))。
// root_distance(v): 成分の根からvへの重みの総和。O(log(n+1))。
// distance(u,v): uとvを結ぶパスの重みの総和。別成分ならnullopt。O(log(n+1))。
//   Tはboolを除く64ビット以下の整数型。計算は型幅で周回する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
