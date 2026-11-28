// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/directed_return_distances.md
//
// API一覧
// directed_return_distances(n,edges): 各頂点から1辺以上通って同じ頂点に戻る最短距離を返す。
//   ReturnDistanceArc{from,to,weight}。頂点は0始まり、重みはuint64_t。自己辺・多重辺・重み0も可。
//   戻れないか、最短距離がuint64_tの上限を超える頂点はnullopt。
//   強連結成分が単一の有向閉路なら一周の和で済む。残りの各成分で始点別にDijkstraを行う。
//   成分cの頂点数・内部辺数をn_c,m_cとし、最悪O(n+m+Σ n_c*m_c*log(m_c+1))時間、O(n+m)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
