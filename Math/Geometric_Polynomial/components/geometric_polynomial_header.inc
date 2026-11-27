// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/geometric_polynomial.md
//
// API一覧
// evaluate_geometric_polynomial(coefficients,a,r,count): f(a*r^i)をi=0..count-1の順で返す。
// interpolate_geometric_polynomial(values,a,r): f(a*r^i)=values[i]を満たす次数n-1以下のn係数。点が重複するとnullopt。
// 法998244353、係数は定数項から。入力係数数と値数の上限は各2^20。r^0=1。空の係数列は零多項式。
// 評価は係数数nと出力数mに対しO((n+m)log(n+m+1))時間・O(n+m+1)領域。補間はO(n log(n+1))時間・O(n+1)領域。
