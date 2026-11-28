// SPDX-License-Identifier: CC0-1.0
// 参考（橋でつないだ成分を保つ考え方）: https://doi.org/10.1007/BF01758773
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/incremental_bridges.md
//
// API一覧
// IncrementalBridges(n): 0始まりの n 頂点、辺なし。有向辺の追加や削除は扱わない。
// add_edge(u,v): 無向辺を追加し入力順の辺IDを返す。自己辺・多重辺可。
// is_bridge(id)/bridge_count(): 現時点で橋か/橋の本数。O(1)。
// size()/edge_count()/component_count()/block_count(): 頂点数/辺数/連結成分数/2辺連結成分数。O(1)。
// same_component(u,v)/same_two_edge_component(u,v): 連結か/任意の辺を1本除いても連結か。償却 O(α(n+1))。
// reserve_edges(m): 辺の判定用領域を m 本分予約できる。
// m 回追加と q 回の成分クエリで O((m+q+n log(n+1)) α(n+1)) 時間、O(n+m) 領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
