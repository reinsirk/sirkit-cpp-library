// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/superincreasing_path_queries.md
//
// API一覧
// SuperincreasingPathQueries<T>(n, edges): 辺iの長さが先行辺の長さの総和より大きい無向グラフを前処理。
//   SuperincreasingPathEdge<T>{from,to,value} は0始まりの端点と、長さとは別の集計値。
//   構築 O(n log(n+1)+m(log(m+1)+log(n+1))) 時間、O(n log(n+1)+m) 領域。
// path_sum(u,v): u-v最短路に含まれる value の和。path_sum_without_edge(u,v,id): 辺idだけ除外して同じ計算。
//   到達不能は std::nullopt。同じ頂点間は T{}。T{} は加法単位元。+,- が使え、加法は可換。
//   クエリ O(log(n+1)) 時間。自己辺と多重辺可。費用が 2^i のときは value=2^i mod P を使える。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
