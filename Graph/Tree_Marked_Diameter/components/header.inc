// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_marked_diameter.md
//
// API一覧
// TreeMarkedDiameter(n, edges, initially_marked=false): 頂点0..n-1の無向木。辺を各1回渡す。
// B=ceil(n/32)としてO(n+B log(B+1))時間・領域。n=0も可。
// size()/marked(v): 頂点数/マーク状態。各O(1)。
// set_marked(v, marked=true)/toggle(v): マークを更新。toggleは更新後の状態を返す。各O(log(n+1))時間。
// diameter(): 全マークの直径{first, second, distance}。O(1)。
// subtree_diameter(v): 頂点0を根としたvの部分木のマークに限定した直径。O(log(n+1))時間。
// 直径の対象が空ならnullopt。1頂点なら両端点は同じ。最長の組が複数あればそのうちの一組。
// farthest(v): vから最も遠いマーク済み頂点と距離{vertex, distance}。マークなしならnullopt。O(1)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
