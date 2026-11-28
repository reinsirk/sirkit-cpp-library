// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/near_tree_distances.md
//
// API一覧
// NearTreeDistances(n, edges): 無向・非重み付きグラフを前処理。多重辺・自己辺も可。
//   成分iの頂点数をn_i、辺数をm_i、余分な辺数をk_i=m_i-n_i+1とする。
//   時間O(sum_i((k_i+1)(n_i+m_i)))、領域O(m+sum_i((k_i+1)n_i))。
// size(): 頂点数。O(1)。
// distance(u,v): 最短路の辺数。非連結なら-1。同じ成分iならO(k_i+log(n_i+1))、非連結ならO(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
