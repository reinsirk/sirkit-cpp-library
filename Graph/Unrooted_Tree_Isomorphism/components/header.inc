// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/unrooted_tree_isomorphism.md
//
// API一覧
// unrooted_trees_equal(n,first,second): 2つのn頂点の無向木の同型判定。n=0の両空木は同型。
// tree_leaf_deletion_matches(n,tree,smaller): n頂点の木から葉1つを消して、n-1頂点の木と同型にできる葉を昇順に返す。n>=2。
//   頂点は0始まり。無向辺をpair<int,int>で1回渡す。両関数とも n<=INT_MAX/4。
//   根の候補を重心に絞り、部分木の同型分類を共有する。衝突のない決定的な判定。O(n log(n+1))時間、O(n)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
