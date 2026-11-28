// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polynomial_recurrence.md
// 参考（多項式係数の漸化式）: https://atcoder.jp/contests/abc222/editorial/2742
//
// API一覧
// polynomial_recurrence_term(initial,coefficients,index): 法998244353で漸化式の第index項を求める。
// d=initial.size()>=1、Σ_{j=0}^d P_j(t)*a_(t+j)=0。coefficients[j]はP_jの定数項順のint64_t係数。
// initialはa_0..a_(d-1)。indexはuint64_t。更新が要る場合、使う全tでP_d(t)!=0。違反はdomain_error。
// 形状不正はinvalid_argument。正規化後の多項式次数は256以下（項更新を行う場合）。
// 更新長L=index-d+1、行列次元s=d+1でpolynomial_matrix_interval_productと同じ計算量。
// index<dは初期値をO(1)で返す。係数列と初期値は変更しない。
