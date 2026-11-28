// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lower_bound_flow.md
//
// API一覧
// LowerBoundFlow<Cap=std::int64_t>(n): n頂点の有向グラフ。頂点0..n-1。
// add_edge(from,to,lower,upper): 0<=lower<=upperの辺を追加し、入力順の辺番号を返す。償却O(1)。
// size()/edge_count(): 頂点数/辺数。O(1)。
// circulation(): 全頂点で流量を保存する流れ。入力順の辺流量を返す。
// min_flow(s,t,limit=Capの上限)/max_flow(s,t,limit): [0,limit]で最小/最大のs-t流。
//   Flow{value, edge_flow}を返す。sとtは異なる頂点。解がなければnullopt。
//   各頂点の入出下限和（自己辺を除く）と必要流量の合計はCapで表せること。
//   解を求める呼び出しは最大流を高々2回とO(n+m)の追加処理。領域O(n+m)。辺を保持したまま再計算できる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
