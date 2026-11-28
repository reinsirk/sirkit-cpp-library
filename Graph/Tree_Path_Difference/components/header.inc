// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_path_difference.md
//
// API一覧
// TreePathDifference<T=int64_t>(n,edges,root=0): 無向森を用意。各辺を pair<int,int> で1回渡す。
//   頂点は0始まり。構築・領域 O(n+B log(B+1))、B=ceil(n/32)。
// add_vertex_path(u,v,delta): 両端を含むパス上の頂点に delta を加算。O(1)。
// add_edge_path(u,v,delta): パス上の辺に delta を加算。O(1)。
//   どちらも別成分なら加算せず false。同一成分なら true。
// vertex_values()/edge_values(): 頂点番号順/入力辺順の累計。取得後も加算できる。O(n) 時間・追加領域。
//   Tはboolを除く64ビット以下の整数型。加減算は型幅で周回する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
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
