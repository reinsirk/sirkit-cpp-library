// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/difference_constraints.md
//
// API一覧
// DifferenceConstraint{from,to,upper}: x[to]-x[from]<=upper。頂点0始まり。多重・自己制約可。
// solve_difference_constraints(n,constraints,anchor=0,inf=2^60): 整数解があるか判定し、原点を anchor に置く。
//   DifferenceConstraintsResult{feasible,values,bounds}。feasible なら values は一つの解で values[anchor]=0。
//   bounds[v].lower/upper は x[v]-x[anchor] が取れる最小/最大。対応する端が非有界なら nullopt。
//   矛盾する場合は feasible=false で2配列とも空。n>=1、|upper|<=10^9。
//   有限距離は (-inf,inf) に入ること。O(n(n+m)) 時間、O(n+m) 領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
