// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bracket_sequence.md
//
// API一覧
// BracketSequence(text): 1種類の括弧列を持つ。構築は長さnに対してO(n+1)時間・領域。
// size(): 長さ。get(index): 指定位置の括弧。各O(1)。
// set(index,symbol): 1文字を置換。swap(first,second): 2箇所の括弧を交換。各O(log(n+1))。
// query(left,right): 半開区間のmatched_pairs・unmatched_open・unmatched_closeを求める。O(log(n+1))。
// [BracketSummary] matched_length(): 対応が取れた文字数。balanced(): 正しい括弧列ならtrue。
// minimum_insertions(): 正しい括弧列に変えるための挿入数。各O(1)。
// first_unmatched_close(left): [left,n)を左から照合したときに余る最初の閉じ括弧。なければn。
// last_unmatched_open(right): [0,right)を右から照合したときに余る最後の開き括弧。なければ-1。各O(log(n+1))。
// 0始まりで添字はint。入力は括弧'('と')'の2文字だけ。長さはINT_MAX以下。
