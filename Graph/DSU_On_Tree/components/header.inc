// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dsu_on_tree.md
//
// API一覧
// DsuOnTree(parents): 親配列の森を前処理。頂点は0始まり。根の親は-1。O(n)時間・領域。
// DsuOnTree(intervals): 構築済みのSubtreeIntervalsを共有して前処理。O(n)時間・追加領域。
// size(): 頂点数。O(1)。
// run(add,remove,answer): answer(v)でvの部分木全体が登録されるようにadd(u), remove(u)を呼ぶ。
//   呼び出し前後の集合は空。頂点ごとにanswerを1回呼ぶ。走査順は規定しない。
//   add/removeは逆操作。コールバックがO(1)なら O(n log(n+1)) 時間、O(1) 追加領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
