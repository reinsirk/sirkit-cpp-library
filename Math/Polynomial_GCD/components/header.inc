// SPDX-License-Identifier: CC0-1.0
// 参考（half-GCDの分割統治）: https://arxiv.org/abs/2212.12389
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polynomial_gcd.md
//
// API一覧
// [PolynomialBezout]
// gcd, first, second: 拡張GCDの結果。first*a + second*b = gcd。
// polynomial_extended_gcd(a,b): 法998244353でモニックGCDとBézout係数。両方0なら全て0。
// polynomial_inverse_mod(value,modulus): value*逆元=1 mod modulus。次数はmodulus未満、逆元がなければnullopt。
// 法多項式が非零定数なら逆元の代表元は0。零多項式を法とする場合はdomain_error。
// Nは末尾ゼロを除いた大きい方の係数数、Lは入力の係数数の合計。N<=2^21でO(L+N log²(N+1))時間、O(L)領域。
