// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/exponential_coefficient_transform.md
// 参考（Lagrange反転の適用）: https://atcoder.jp/contests/agc076/editorial/14958
//
// API一覧
// exponential_coefficient_transform(series, step, precision): n番目の係数が[x^n]series(x)*exp(step*n*x)になる級数。
// inverse_exponential_coefficient_transform(series, step, precision): 同じstepの指数が次数で変わる係数変換の逆。
// 係数とstepは法998244353。精度Nは0..2^18、戻り値の係数数はN。
// O(N log²(N+1))時間、O(N log(N+1))領域。step=0なら切り詰め・0埋めだけ。

// SIRKIT_BUNDLED_DEPENDENCIES

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
