// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bipartite_matching_analysis.md
//
// API一覧
// analyze_bipartite_matching(left,right,edges): 最大マッチングにおける全辺と全頂点の役割を調べる。
//   edgesは(left_id,right_id)で、左右別々に0始まり。多重辺のIDも入力順で区別する。
//   matching_sizeは最大サイズ。edge_roleは入力辺順のforbidden/optional/mandatory。
//   左右のcan_be_freeは、該当頂点を未対応にする最大マッチングがあるか。
//   n=left+right。最大マッチング1回とO(n+m)の追加処理、領域O(n+m)。


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
