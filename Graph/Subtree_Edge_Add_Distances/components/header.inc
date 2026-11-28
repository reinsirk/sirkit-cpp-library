// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subtree_edge_add_distances.md
//
// API一覧
// SubtreeEdgeAddDistances<T=int64_t>(n,edges,root=0): 固定の無向森。Edge{from,to,weight} を各辺1回渡す。
//   指定根のない成分は最小頂点を根とする。構築・領域 O(n+B log(B+1))、B=ceil(n/32)。
// size(): 頂点数。O(1)。
// add_subtree(v,delta): vの部分木内の全辺に加算。親からvへの辺は対象外。O(log(n+1))。
// root_distance(v): 成分の根からvまでの辺重みの総和。O(log(n+1))。
// distance(u,v): パス上の辺重みの総和。異なる成分なら nullopt。O(log(n+1))。
//   頂点は0始まり。T は bool を除く64ビット以下の整数型。加減算は型幅で周回する。


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
