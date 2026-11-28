// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polynomial_moments.md
// 参考（指数関数との合成と競技での用途）: https://nyaannyaan.github.io/library/fps/composite-exp.hpp
//
// API一覧
// weighted_power_sums(points,weights,n): k=0..n-1のΣ weights[i]*points[i]^kをFormalPowerSeriesで返す。0^0=1。
// exponential_composition(f,scale,n,shift=0): 多項式f(exp(scale*x)+shift)の先頭n係数を返す。
// 法998244353。整数ベクトルは負数・uint64_tも可。nとfの係数数は2^22以下、点と重みの個数は同じ。
// 入力m要素/係数、出力n係数に対しO((m+n+1)log^2(m+n+2))時間・O(m+n+1)領域。
