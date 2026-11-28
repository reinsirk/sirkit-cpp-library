// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lexicographic_concatenation.md
// 参考（並べ替えの原理）: https://info.atcoder.jp/entry/algorithm_lectures/lex_min_string_concatenation
//
// API一覧
// lexicographic_concatenation_order(words): 連結結果が辞書順最小となる添字の並びを返す。
// second_lexicographic_concatenation(words): 全順列での連結結果を重複ごと数え、辞書順で2番目を返す。2本未満はnullopt。
// 符号なしバイト順。空文字列も可。n個、総長LでO(n log(n+1)+L)時間、O(n+L+1)領域。L<=INT_MAX-2。
