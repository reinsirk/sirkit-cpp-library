// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/offline_prefix_lcp_sum.md
//
// API一覧
// PrefixLcpAppend{index,byte}: 0始まりの文字列indexの末尾に1バイト追加する。
// PrefixLcpQuery{index}: 文字列indexと現在の各文字列との最長共通接頭辞長の総和を照会する。
// PrefixLcpOperation: 上の操作のvariant。空文字列や同じ内容の入力も可。総和に照会先自身も含める。
// offline_prefix_lcp_sum(initial,operations): 操作をまとめて受け取り、照会の答えを順に返す。
// 初期本数N、合計長S、操作数Q、作るTrieの頂点数MとしてO(N+S+(Q+1)log(M+1))時間・O(N+M+Q)領域。バイト集合は固定。
// NとMはINT_MAX以下。同梱Fenwick木のAPIはDocument/fenwick_tree.mdを参照。
