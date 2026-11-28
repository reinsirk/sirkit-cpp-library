// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_edge_color_path_counts.md
//
// API一覧
// tree_edge_color_path_counts(n,edges): 木のパスに現れる辺色が1種類、2種類の頂点対数をそれぞれ返す。
//   TreeColorEdge{from,to,color}。頂点は0始まり、色は任意のint64_t。
// TreeEdgeColorPathCounts{one_color,two_colors}: 異なる2頂点を順序なしで数える。
//   重心を通るパスだけを左右の枝の色集合から数える。O(n log²(n+1))時間、O(n)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
