// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_walk_orientation.md
// 数学的参考: https://img.atcoder.jp/agc025/editorial.pdf
//
// API一覧
// orient_tree_walks(n, edges, walks): 0 始まりの無向木と、移動する各パスの両端を渡す。
// TreeWalkOrientationResult{covered_directions,reversed}:
//   各木辺について使う向きの数 0..2 を足した最大値と、入力順の反転フラグを返す。
//   同じパスを何度渡してもよい。両端が同じパスは辺を使わない。空の木には空のパス列を渡す。
//   n<=(INT_MAX-1)/2、パス数 m として O(n+m+1) 時間・領域。非再帰。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
