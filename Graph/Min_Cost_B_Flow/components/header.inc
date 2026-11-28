// SPDX-License-Identifier: CC0-1.0
// 参考（費用スケーリング）: https://doi.org/10.1287/moor.15.3.430
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/min_cost_b_flow.md
//
// API一覧
// BFlowEdge{from,to,lower,upper,cost}: 流量範囲[lower,upper]の有向辺。費用や下限に負を許す。
// min_cost_b_flow(supply,edges): 各頂点で出る流量−入る流量=supplyを満たす最小費用解を返す。
// BFlowResult{flow,potential}: 入力順の辺流量と頂点ポテンシャル。実現不能はnullopt。
// 逆向きも含む辺範囲と内部の費用・流量の演算はint64_tに収める。総費用の型は利用側で選ぶ。
// n頂点、m辺、辺費用の最大絶対値C。最大流と費用スケーリングでO((n+1)^2(n+m+1)(1+log(1+nC)))時間・O(n+m+1)領域。
// 下限・上限や端点の入力検査はSKIP_RUNTIME_CHECKSで省ける。添字・算術の上限検査は維持する。
// 同梱のmf_graph<Cap>: 最大流のAPIと計算量はDocument/max_flow.mdを参照。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <new>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
