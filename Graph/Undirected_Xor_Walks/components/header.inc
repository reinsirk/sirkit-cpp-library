// SPDX-License-Identifier: CC0-1.0
// 参考（数学的構成）: https://atcoder.jp/contests/abc451/editorial/18047/print
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/undirected_xor_walks.md
//
// API一覧
// XorWalkEdge{from,to,weight}: 0始まりの両端点とuint64_tの辺重み。
// UndirectedXorWalks(n,edges): 無向グラフの連結成分とXOR基底を作る。O(64(n+m))時間、O(n+m+64c)領域。
//   cはXORが非零となる閉じた歩道を持つ連結成分の数。
// min_xor(u,v): uからvへの歩道の最小XOR。到達不能ならnullopt。O(1)。
// max_xor(u,v): 最大XOR。到達不能ならnullopt。O(64)。
// attainable(u,v,x): XORがxとなる歩道が存在するか。O(64)。
// count_pairs_at_most(x): 最小XORがx以下になる頂点組u<vの数。O(n(log(n+1)+64))時間、追加O(n)領域。
//   歩道は頂点・辺の再訪と、同じ頂点での空の歩道を許す。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
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
