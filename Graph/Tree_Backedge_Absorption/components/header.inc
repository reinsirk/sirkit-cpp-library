// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_backedge_absorption.md
//
// API一覧
// TreeBackedgeAbsorption(parents, backward_edges): 親から子へ進む根付き木に、葉以外から真の祖先への辺を加える。
//   parents[v]=-1 は唯一の根。頂点は0始まり。重複辺は選択肢に別々に数える。構築 O(n+m)。
// expected_steps<Field>(): 根から各出辺を等確率で選び、葉に着くまでの移動回数の期待値。
// expected_cost<Field>(costs): 頂点vを出発する度に払う costs[v] の合計の期待値。
//   Field は0,1と辺本数を表せる体。消去中の係数が0なら std::nullopt を返す。
//   n頂点、追加m辺で O((n+m) log²(n+m+1)) 回の体演算、うち除算は O(n)。領域 O(n+m)。
// 部分木内の頂点を消去し、祖先に戻るときの係数と部分木滞在中の期待コストをまとめる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
