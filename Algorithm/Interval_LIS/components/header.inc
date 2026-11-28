// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/interval_lis.md
//
// API一覧
// LISInterval{left,right}: その位置で選べる整数の閉区間[left,right]。端点はint64_t、left<=right。
// interval_lis_tails(intervals): 添字順に選び、狭義増加な部分列を作る。長さi+1で作れる最小末尾値を結果[i]に返す。
// 返す配列の長さが最大のLIS長。空の入力は空配列。区間数はINT_MAX以下。
// nを区間数として、期待O((n+1)log(n+2))時間、O(n+1)領域。同梱の暗黙TreapはDocument/implicit_treap.mdを参照。
