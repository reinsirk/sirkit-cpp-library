// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/walk_weight_ranges.md
//
// API一覧
// WalkWeightEdge{from,to,weight}: 0始まりの端点とint32_tの符号付き辺重み。
// walk_weight_ranges(n,edges,source): 始点から各頂点へ着く歩道の最小・最大重みを調べる。
//   頂点・辺を再訪でき、始点での空の歩道も許す。O(n+m)時間・領域。
// WalkWeightRange{reachable,unbounded,minimum,maximum}: 到達可否、一方以上の非有界性、最小・最大値。
//   非有界または到達不能ならminimumとmaximumは0。有限な値はint64_t。


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
