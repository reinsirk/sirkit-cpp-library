// SPDX-License-Identifier: CC0-1.0
// 参考（奇数次数頂点のペアリング）: https://doi.org/10.1007/BF01580113
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/chinese_postman_route.md
//
// API一覧
// PostmanEdge{from,to,weight}: 0始まりの端点と非負の uint64_t 重みを持つ無向辺。自己辺・多重辺可。
// minimum_postman_route(n, edges, start=-1, finish=-1): 全辺を1回以上通る最小費用ルート。
//   start=-1 は開始点を自動選択。finish=-1 は開始点に戻る。ルートがなければ std::nullopt。
//   PostmanRoute{cost,vertices,edge_ids} は費用、通過頂点列、対応する入力辺ID列。
//   奇数次数とすべき端点を補正した後のペアリング対象が k 頂点。k<=22。
//   O(n+m + k(n+m) log(m+1) + k 2^k) 時間、O((k+1)n + m + 2^k) 領域。
//   kが上限を超えれば length_error、費用が uint64_t を超えれば overflow_error。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
