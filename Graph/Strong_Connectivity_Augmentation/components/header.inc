// SPDX-License-Identifier: CC0-1.0
// 参考（縮約DAGの端点ペアと探索の停止条件）: https://terpconnect.umd.edu/~raghavan/preprints/stc.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/strong_connectivity_augmentation.md
//
// API一覧
// strong_connectivity_augmentation(n, edges): 有向グラフを強連結にする追加辺 (from,to) の最小本数の集合。
//   頂点は0始まり。辺は入力順の組。自己辺、多重辺、孤立頂点可。
//   空グラフまたはすでに強連結なら空集合。候補はいずれも入力にない辺。
//   縮約DAGで入次数0と出次数0の成分を集めてつなぐ。O(n+m) 時間・領域、非再帰。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
