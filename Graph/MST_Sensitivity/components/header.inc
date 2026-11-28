// SPDX-License-Identifier: CC0-1.0
// 参考（1辺の変更と代替辺）: https://atcoder.jp/contests/abc301/editorial/6353
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/mst_sensitivity.md
//
// API一覧
// minimum_spanning_forest_sensitivity(n,edges): 各辺を1本だけ除外または強制したときの交換辺を求める。
// SpanningForestSensitivity{forest,exchange}: 選んだ森の辺番号と、全入力辺に対応するoptionalの交換辺番号。
// 森に含まれるeでは、eを除外するとき代わりに入れる最小重みの非木辺。なければnullopt。
// 森に含まれないeでは、eを強制するとき除く木辺。自己辺は強制できないのでnullopt。
// 同じ重みの交換候補では入力番号が小さいもの。非連結・多重辺・負の重みも可。重みの総和は計算しない。
// O(n log(n+1)+m(log(m+1)+log(n+1)))時間・O(n log(n+1)+m)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
