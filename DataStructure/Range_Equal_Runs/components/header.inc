// SPDX-License-Identifier: CC0-1.0
// 参考（問い合わせの用途）: https://atcoder.jp/contests/aalc001/tasks/aalc001_f
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_equal_runs.md
//
// API一覧
// RangeEqualRuns<T>(values): 長さnの列を保持。空構築可。O(n+1)時間・領域。n<=INT_MAX。
// size()/empty(): 長さと空判定。O(1)。get(i): 要素のコピー。O(1)。
// set(i,value): 一点代入。O(log(n+1))。比較を先に終え、例外時は元の状態を保つ。
// query(l,r): 半開区間のEqualRunLengths{length,prefix,suffix,longest}。O(log(n+1))。
// all(): 全区間の同じ集計。O(1)。空区間の長さはすべて0。
// prefix/suffix/longestは同じ値の連続長。Tはコピー構築可能、例外を投げずに交換可能、==が同値関係。
// 添字は0始まり。検査有効時は範囲外でout_of_range。コピーは独立、ムーブ元は空。
