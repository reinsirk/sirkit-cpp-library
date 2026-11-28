// SPDX-License-Identifier: CC0-1.0
// 参考（独立集合の分割による彩色数え上げ）: https://epubs.siam.org/doi/10.1137/070683933
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/chromatic_polynomial.md
//
// API一覧
// chromatic_polynomial(n, edges): 無向グラフの彩色多項式を法998244353で返す。定数項からn次の順。
// 同色の頂点を隣接させずにk色で塗る方法の数は、返した多項式のkでの評価値。
// nは0..20。多重辺を許す。自己辺があれば全係数0、頂点がなければ{1}。
// m辺に対してO((n+1)^2*2^n+m)時間・O((n+1)*2^n)領域。
// 同梱の subset_convolution / set_series_exp / set_series_log: 法998244353の集合冪級数演算。
// 長さ2^nの列に対し、時間O((n+1)^2*2^n)・領域O((n+1)*2^n)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
