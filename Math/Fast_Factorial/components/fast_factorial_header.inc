// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/fast_factorial.md
//
// API一覧
// factorial_mod_998244353(n): n! mod 998244353を返す。nが法以上なら0。
// factorials_mod_998244353(queries): 各n!の剰余を入力順で返す。空入力は空。
// Wilsonの定理を使い、nと法-1-nのうち小さい方へ変換する。その最大値をMとする。
// Mが小さいときはO(M+q log(q+1))。他は幅Bの積を補間で求め、時間は
// O((B+M/B)log(B+M/B+1) + q log(q+1) + min(M,qB))、領域はO(B+M/B+q)。
