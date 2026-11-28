// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tridiagonal_matrix_power.md
// 参考（三重対角行列の余因子と転置分割統治）: https://atcoder.jp/contests/arc228/editorial/24856
//
// API一覧
// tridiagonal_power_first_column(exponent,lower,diagonal,upper): 法998244353の三重対角行列の累乗の第0列。
// n=diagonal.size()、下・上対角の長さはmax(0,n-1)。lower[i]=T[i+1][i]、upper[i]=T[i][i+1]。
// 係数はint64_tで負も可。指数はuint64_t。空行列は空の列、指数0は第0単位ベクトル。
// s=min(n,exponent+1)が2^20以下。長さ不一致はinvalid_argument、上限超過はlength_error。
// O(n+s log(s+1)(log(s+1)+log(exponent+2)))時間、O(n+s(log(s+1)+log(exponent+2)))領域。
