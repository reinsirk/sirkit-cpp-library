// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_distance_sums.md
//
// API一覧
// TreeDistanceSums<T>(n,edges,weights={},root=0): 0始まりの重み付き木。空のweightsは全頂点を0で初期化。
//   TreeDistanceSumEdge<T>{from,to,length=1}。d(u,v)は木のパスにある辺長の和。構築O(n)時間・領域。
// add(v,delta): 頂点重みW[v]にdeltaを加える。sum_at(v): Σ W[r]*d(r,v) を返す。
// sum_path(u,v): u..vの両端を含むすべての頂点xについてsum_at(x)の総和を返す。
//   更新と照会はO(log²(n+1))時間。Tは加減乗算と整数からの変換を持つ環の値。
//   組み込み整数Tは型幅で周回。区間照会は遅延伝播を行う。
//   LCAまでの辺長を、根からxまでの辺長と各子部分木重みの積の和で集約する。
//   根からの経路に集約を重ねると、深さdの辺はdepth(x)-d+1回現れる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
