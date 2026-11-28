// SPDX-License-Identifier: CC0-1.0
// 参考: https://atcoder.jp/contests/abc330/editorial/7752
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/point_set_mex.md
//
// API一覧
// PointSetMex<T=int>(values): 整数列を所有して準備する。長さnは固定。O(n+1)時間・領域。
// PointSetMex<T>(): 空列を用意する。O(1)。コピーは独立。ムーブ元は空列。
// size(): 要素数。get(i): 0始まり添字iの値。mex(): 列にない最小の非負整数。各O(1)。
// set(i,value): 位置iを更新。O(1+log_64(n+1))。負数とnより大きい値も保持し、mexの計算から外す。
// get/setはi<size()が前提。検査有効時の範囲外はstd::out_of_range。
