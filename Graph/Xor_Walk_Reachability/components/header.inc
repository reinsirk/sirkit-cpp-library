// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/xor_walk_reachability.md
//
// API一覧
// XorWalkEdge{from,to,weight}: 0始まりの両端点とuint64_tの辺重み。
// XorWalkReachability(n,edges,source,directed=true): 始点からのXOR付き歩道を調べる。
//   辺重みのビット幅をBとして、O((n+m)2^B)時間、O(n2^B+m)領域。Bが小さいグラフ用。
// reachable(v,x): XORがxになる歩道が存在するか。O(1)。
// min_xor(v)/max_xor(v): vに着く歩道の最小/最大XOR。到達不能ならnullopt。O(1)。
//   同じ頂点・辺を再訪でき、始点では空の歩道も許す。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
