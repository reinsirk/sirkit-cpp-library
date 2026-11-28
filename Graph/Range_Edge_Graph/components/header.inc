// SPDX-License-Identifier: CC0-1.0
// 参考（セグメント木での区間辺の圧縮）: https://atcoder.jp/contests/abc414/editorial/13415
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_edge_graph.md
//
// API一覧
// RangeEdgeGraph(n): 頂点0..n-1の有向グラフ。定数の辺費用を使う層0を用意する。O(n)時間・領域。
// add_layer(departure,arrival): 頂点ごとの出発・到着費用を持つ層を追加し、層番号を返す。O(n)。
// add_range_edge(l,r,L,R,w,layer=0): [l,r)のuから[L,R)のvへ w+departure[u]+arrival[v] の辺を追加。
// 空区間があれば何もしない。補助頂点は高々2個、辺の追加O(log(n+1))、辺列拡張を含め償却。
// add_edge(u,v,w): 元の頂点間の有向辺を1本追加。償却O(1)。重みの型はstd::int64_t。
// size()/vertex_count(): 元の頂点数/補助頂点込みの頂点数。edges(): (始点,終点,重み)の辺列へのconst参照。各O(1)。
// graph(): WGraph形式に変換して返す。拡張後の頂点数Vと辺数Eに対しO(V+E)時間・領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
