// SPDX-License-Identifier: CC0-1.0
// Bernoulli数の母関数: https://dlmf.nist.gov/24.2.E1
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polynomial_prefix_sum.md
//
// API一覧
// PolynomialPrefixSum(f): g(0)=0、g(x+1)-g(x)=f(x)を満たす多項式を返す。
// fがn係数ならgはn+1係数。n≤2^22、法998244353。O(1+n log(n+1))時間・O(1+n)領域。
