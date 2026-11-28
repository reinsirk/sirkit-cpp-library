// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_steiner_subtree.md
//
// API一覧
// DynamicSteinerSubtree(n,edges,root=0): 頂点0..n-1の単純無向木。辺を1回ずつ指定し、最初はすべて未マーク。
// B=ceil(n/32)としてO(n+B log(B+1))時間・領域。
// set_marked(v,marked=true): マークの有無を設定。toggle(v): 反転して新しい状態を返す。各O(log(n+1))。
// steiner_vertices(): 全マーク済み頂点を含む最小連結部分木の頂点数。マークなしなら0。O(1)。
// steiner_vertices(v,root): rootを根としたvの部分木内のマーク済み頂点に限定。O(log(n+1))。
// marked(v)/size(): マークの有無/木全体の頂点数。各O(1)。
// コピーした木のマークは独立。コピーはO(n)、ムーブ構築はO(1)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <span>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
