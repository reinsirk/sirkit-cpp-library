// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_add_range_gcd.md
// 参考（階差を利用する方法）: https://atcoder.jp/contests/arc017/tasks/arc017_4
//
// API一覧
// RangeAddRangeGcd<T=int64_t>()/(values): 空列またはTの列から構築。Tは符号付き整数型。要素数はintに収まること。
// size(): 要素数。O(1)。get(i): 現在の値。O(log(n+1))。
// add(l,r,delta): 半開区間[l,r)に加算。加算後の各値はTに収まること。空区間は何もしない。
// gcd(l,r): 半開区間[l,r)の非負の最大公約数をTに対応する符号なし型で返す。空区間は0。
// Mを値・階差の絶対値の上限とし、構築O(n(1+log(M+1)))時間・O(n)領域、add/gcdはO(log(n+1)(1+log(M+1)))時間。
