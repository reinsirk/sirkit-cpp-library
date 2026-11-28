// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polynomial_recurrence_fit.md
// 参考（P-recursive数列の先頭項からの復元）: https://atcoder.jp/contests/abc222/editorial/2742
//
// API一覧
// fit_polynomial_recurrence(terms, order, max_degree):
// 与えた区間でΣ_{j=0}^order P_j(t)*terms[t+j]=0を満たす多項式を法998244353で1組返す。
// 各P_jは定数項順にmax_degree+1係数。P_orderが非零多項式となる解がなければnullopt。
// 1<=order<terms.size()。形状不正はinvalid_argument、寸法が大きすぎればlength_error。
// N項、K=(order+1)(max_degree+1)変数に対しO(NK²+K log Mod)時間・O(N+K²)領域。
// 入力区間外で成立するか、P_orderが評価する範囲で非零かは利用側で確認する。
