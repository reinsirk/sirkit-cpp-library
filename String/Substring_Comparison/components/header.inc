// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/substring_comparison.md
//
// API一覧
// SubstringComparison(text): O(n+1)時間・領域で前処理。入力の保存は不要。長さはINT_MAX-2以下。
// size(): 元の文字列のバイト数。
// common_prefix(first,second): 2つの接尾辞の最長共通接頭辞の長さ。末尾位置は空の接尾辞。
// equal(first,second,length): 指定位置から同じ長さの部分文字列が一致するか。
// compare(first,first_length,second,second_length): 部分文字列を辞書順で比較。バイトは符号なしの順序。
// 添字は0始まり。開始位置と長さは元の文字列に収まること。各照会はO(1)、コピー構築は前処理結果を共有。
