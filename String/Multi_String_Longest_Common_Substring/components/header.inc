// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/multi_string_longest_common_substring.md
//
// API一覧
// LongestCommonSubstring{length, starts}: 共通部分文字列の長さと、入力の各文字列での0始まりの開始位置。
// longest_common_substring(texts): すべての入力に現れる最長の連続部分文字列を求める。最長が複数ならいずれかを返す。
// 入力が空か空文字列を含む場合は長さ0・各開始位置0。1本の場合はその全体。
// 2本以上を索引化するときは文字列長の合計と本数の和がINT_MAX-1未満。各O(N+k+1)時間・領域（Nは長さの合計、kは本数）。
