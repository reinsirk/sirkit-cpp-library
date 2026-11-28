// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/affine_walk_ranges.md
//
// API一覧
// AffineWalkEdge{from,to,reflect,shift}: 有向辺。reflect=true は x←-x+shift、false は x←x+shift。
// affine_walk_ranges(n, edges, source): 座標0で始め、各頂点で取り得る座標の最小値・最大値を計算。
//   頂点は0始まり、頂点・辺の再訪可。始点での空の歩道も含める。|shift|<=INT32_MAX、n<=INT_MAX/2。
//   WalkWeightRange{reachable,unbounded,minimum,maximum}。到達不能または非有界なら極値欄は0。
//   符号の状態を2種類に分け、加算だけの歩道に直す。O(n+m) 時間・領域。


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
