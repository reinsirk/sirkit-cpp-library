// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bipartite_coloring.md
//
// API一覧
// bipartite_coloring(n, edges): 無向グラフを二部に分ける。頂点0..n-1。自己辺・多重辺可。
//   二部グラフなら color[v] は 0 または 1、odd_cycle は nullopt。
//   二部グラフでなければ color は空、odd_cycle に頂点列と入力の辺番号列を返す。
//   O(n+m)時間・領域。
// BipartiteColoring::is_bipartite(): 2色に分けられる場合にtrue。O(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
