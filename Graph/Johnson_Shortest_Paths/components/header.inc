// SPDX-License-Identifier: CC0-1.0
// 参考（Johnson 法）: https://doi.org/10.1145/321992.321993
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/johnson_shortest_paths.md
//
// API一覧
// JohnsonEdge{from,to,weight}: 0始まりの端点と int64_t の重み。自己辺・多重辺可。
// for_each_johnson_distance_row(n,edges,on_row): 全点対の最短距離を始点順に1行ずつ渡す。
//   on_row(source, span<const optional<int64_t>>) の span は呼び出し中だけ有効。未到達はnullopt。
//   負閉路があれば on_row を呼ばず false。それ以外は true。
//   重みの絶対値は INT64_MAX/(4 max(1,n)) 以下。負辺可。
//   時間 O(n(n+m log(m+1)))（コールバックを除く）、追加領域 O(n+m)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
