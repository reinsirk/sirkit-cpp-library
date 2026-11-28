// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/grid_slide_distances.md
//
// API一覧
// grid_slide_distances(grid,start,axes=GridSlideAxes::orthogonal,wall='#'): 全マスまでの最少手数。
// grid_slide_distance(grid,start,target,axes=GridSlideAxes::orthogonal,wall='#'): 指定したマスまでの最少手数。
//   各手で壁に当たる手前まで、同じ方向へ任意の正のマス数だけ進める。
//   orthogonalは縦横、diagonalは斜め。座標は0始まりの(行,列)。
//   壁や到達不能なら-1。全距離の添字はrow*width+column。O(HW)時間・領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
