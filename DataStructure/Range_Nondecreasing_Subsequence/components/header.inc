// SPDX-License-Identifier: CC0-1.0
// 参考（問い合わせの用途）: https://yukicoder.me/problems/no/2992
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_nondecreasing_subsequence.md
//
// API一覧
// RangeNondecreasingSubsequence<A>(symbols): 値が0以上A未満の列を保持。1<=A<=256、n<=INT_MAX。
// 構築はO(A^3*(n+1))時間、O(A^2*(n+1))領域。空構築可。区間は0始まりの半開。
// size()/empty(): 長さ/空か。O(1)。set(i,x): 位置iへxを代入。
// longest(l,r): 最長広義単調増加部分列の長さ。longest(l,r,low,high): 値も閉区間[low,high]に限定。
// replacement_count(l,r): 広義単調増加列にする最少置換回数。上の更新・照会はO(A^3*log(n+1))。
// all(): 列全体の最長長さ。O(A^2)。空区間は長さ0。コピーは独立、ムーブ元は空。
