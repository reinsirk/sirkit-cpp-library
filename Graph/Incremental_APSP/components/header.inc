// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/incremental_apsp.md
//
// API一覧
// IncrementalAPSPEdge{from,to,weight}: 0始まりの端点とuint32_tの非負重み。
// IncrementalAPSP(n,edges={},undirected=false): 初期の全点対最短距離を構築する。
//   初期辺がある場合O(n³+m)時間、なければO(n²)時間。保持領域O(n²)。
// add_edge(from,to,weight): 辺を追加し、既存の辺より小さい重みなら距離を更新する。O(n²)。
//   既存の最短距離以下にできない辺はO(1)で無視する。辺の削除や重みの増加は扱わない。
// distance(from,to): int64_tの最短距離。到達不能ならnullopt。size(): 頂点数。ともにO(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
#include <immintrin.h>
#endif


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
