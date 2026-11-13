// SPDX-License-Identifier: CC0-1.0
// 数学的参考（係数抽出）: https://specfun.inria.fr/bostan/BoMo20.pdf
// 数学的参考（講座）: https://info.atcoder.jp/entry/algorithm_lectures/linearly_recurrent_sequence_kth_term
// 参考（用途・Verify問題）: https://nyaannyaan.github.io/library/fps/nth-term.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/linear_recurrence.md
//
// API一覧
// 以下のcは長さd、aは最初のd項。a[n]=sum(c[j]*a[n-j-1])をn>=dで用いる。
// LinearRecurrenceTerm<T>(a,c,k): 0始まりk項目。O(d² log(k+2))時間・O(d)領域。d=0ならT{0}。
// LinearRecurrenceWeights<T>(c,k): k項目の初期d項への係数。k<dはO(d)、それ以外は上記と同計算量。d=0なら{}。
// LinearRecurrencePrefix<T>(a,c,count): 先頭count項。O(count*(1+d))時間・O(count)領域。
// Tは正確な可換環の加減乗算と0/1構築を提供する。除算は不要。Mintは利用側が用意する。
// 上記uint32_t vector overload: 法998244353で計算し、入力は自動で正規化する。
// uint32_t版TermはBostan–Mori+同梱NTT。L=bit_ceil(2d+1)としてO(L logL log(k+2))時間・O(L)領域。
// uint32_t版Weights/Prefixは汎用版と同計算量。ClangのWeights(d>=16)は標準整数のMontgomery演算を使う。
// TermのNTT長は2^23以下、短い畳み込みは直接計算する。
// a.size()==c.size()が前提。長さ・容量の内部上限は検査省略時も維持する。全APIは入力不変。
// NTTも独立ガードで同梱。NTT/FPS/Berlekamp_Massey.cppと重ねて貼付できる。
