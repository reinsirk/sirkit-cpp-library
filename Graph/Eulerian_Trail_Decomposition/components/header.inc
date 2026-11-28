// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/eulerian_trail_decomposition.md
//
// API一覧
// minimum_eulerian_trail_decomposition<Directed=false>(n, edges):
//   全辺を一度ずつ通る小道のうち、本数が最小となる分解を返す。時間・領域 O(n+m)。
//   頂点は 0 以上 n 未満。edges[i] が辺 ID i。自己辺・多重辺可。
//   Directed=true で有向、既定は無向。辺を含まない成分は結果に含めない。
//   各 EulerianTrail<Directed>::Trail の vertices は頂点列、edges は辺 ID 列。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
