// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_add_weighted_minimum.md
// 参考（Verify問題）: https://atcoder.jp/contests/aalc001/tasks/aalc001_n
//
// API一覧
// RangeAddWeightedMinimum<T=int64_t,Weight=uint64_t>(values,weights)/(weights): 値と重みの列、または値がすべて0の列から構築。O(n)時間・領域。
// add(l,r,delta): 半開区間[l,r)の値にdeltaを足す。重みは固定。O(log(n+1))。
// minimum(l,r)/all_minimum(): 区間/全体の最小値valueと、それを取る要素の重み和weightを返す。空ならnullopt。O(log(n+1))/O(1)。
// size(): 要素数。O(1)。Tは符号付き整数型。値と保留中の加算量、重み和は指定の型に収まること。
