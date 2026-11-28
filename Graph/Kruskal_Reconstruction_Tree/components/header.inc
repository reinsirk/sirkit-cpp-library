// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/kruskal_reconstruction_tree.md
//
// API一覧
// KruskalReconstructionTree(n, edges, order=KruskalOrder::increasing): 無向グラフの併合履歴を森にする。
//   decreasingは重い辺から処理。等重みは入力順。葉は0..n-1、併合ごとに新しい番号を取る。
//   構築はO(n log(n+1)+m log(m+1))時間・領域以下。
// size()/node_count(): 元の頂点数/併合ノードを含む全ノード数。O(1)。
// node(i): Node{parent,left,right,vertex_count,weight}の参照。根の親と葉の子は-1。葉のweightはnullopt。
// merge_node(u,v): 初めて同じ成分に入るノード。v==uならu、非連結なら-1。O(1)。
// merge_weight(u,v): そのときの辺重み。繋がらないか同じ頂点ならnullopt。O(1)。
// component_at(v,weight): 指定の重みまで辺を加えたときの成分を表すノード。O(log^2(n+1))。


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
