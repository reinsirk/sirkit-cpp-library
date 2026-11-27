// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/two_square_sum.md
//
// API一覧
// two_square_sum(n): x*x+y*y=nを満たす非負整数の順序付き組(x,y)を辞書順で返す。
// n=0は{(0,0)}。表現できなければ空。uint64_t全域に対応する。
// 素因数分解の後、p≡1 (mod 4)の各素因数をCornacchia法で分解する。
// t=∏(e+1)をp≡1 (mod 4)の素因数の指数eで計算すると、列挙・整列はO(t log(t+1))時間・O(t)領域。
