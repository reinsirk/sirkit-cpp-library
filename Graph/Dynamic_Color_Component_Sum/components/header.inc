// SPDX-License-Identifier: CC0-1.0
// 参考（重軽分解による同色連結成分の集計）: https://atcoder.jp/contests/abc460/editorial/21024
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_color_component_sum.md
//
// API一覧
// DynamicColorComponentSum(weights, colors, edges): 頂点の重みと色、0始まりの無向木の辺を渡す。構築O(n)時間・領域。
// size()/weight(v)/color(v): 頂点数・頂点の重み・色を返す。O(1)。
// set_color(v,c)/toggle(v): 0または1を設定/反転。toggleは新しい色を返す。
// set_weight(v,x)/add_weight(v,x): 重みを代入/加算する。
// component_sum(v): vと同色の頂点だけで到達できる全頂点（vを含む）の重みの和。
// 更新・成分の集計はO(log²(n+1))。重みと和は64ビット幅で計算する。
// 同梱のHeavyLightDecompositionとfenwick_treeは、各原本のAPI一覧を参照。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
