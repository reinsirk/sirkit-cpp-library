// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_nearest_marked.md
//
// API一覧
// TreeNearestMarked(graph): 頂点0..n-1の単純無向木。最初はマークなし。O(n log(n+1))時間・領域。
// set_marked(v,marked=true): マークの有無を設定。toggle(v): 反転して新しい状態を返す。各O(log^2(n+1))。
// nearest(v): 距離が最小のマーク済み頂点と辺数距離を返す。距離が等しければ最小の頂点番号。
// 戻り値はoptional<Result>、Result{vertex,distance}。マークがなければnullopt。O(log(n+1))。
// marked(v)/size(): マークの有無/頂点数。各O(1)。
// コピーした木のマークは独立。コピーはO(n log(n+1))、ムーブ構築はO(1)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
