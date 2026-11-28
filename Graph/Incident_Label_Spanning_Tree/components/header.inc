// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/incident_label_spanning_tree.md
//
// API一覧
// LabeledSpanningEdge<Label>{from,to,label}: 0始まりの端点とラベルを持つ無向辺。
// incident_label_spanning_tree(required,edges): 全頂点vが required[v] と同じラベルの選択辺に接する全域木。
//   選んだ辺の0始まり入力IDを返す。存在しなければ std::nullopt。多重辺と自己辺可。空グラフなら空列。
//   Label は等値比較可能。O(n+m α(n+1)) 償却時間、O(n+m) 領域。
//   両端を満たす辺で森を作り、満たした頂点から片側の条件を満たす辺を伸ばしてから全域木にする。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
