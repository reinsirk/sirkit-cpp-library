// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/suffix_tree.md
//
// API一覧
// [SuffixTree] 非空接尾辞の圧縮トライ。文字列を所有し、根は頂点0。文字列長nは(INT_MAX-1)/2以下。
// SuffixTree(text): O(n+1)時間・領域で構築。コピーは木を共有する。
// nodes(): Node{parent,depth,suffix_start,first_suffix,last_suffix}の頂点番号順ビュー。根のparentは-1。
// depthは根からの文字数、suffix_startはその頂点で終わる接尾辞の開始位置（なければ-1）。
// first_suffix/last_suffixは辞書順の接尾辞配列内の子孫範囲（半開区間、頂点自身の接尾辞も含む）。
// children(vertex): 子の頂点番号を辺ラベルの辞書順で返すビュー。suffixes(vertex): 子孫の接尾辞の開始位置を辞書順で返すビュー。
// edge_label(vertex): 親からの辺ラベルを参照するビュー。根では空。各ビュー取得はO(1)時間。頂点番号はnodes()の添字。
// longest_nonoverlapping_repeat(): 重ならず2回現れる最長の部分文字列をRepeat{first_start,second_start,length}で返す。
// 該当する非空部分文字列がなければ各値0。O(n+1)時間・追加領域。文字の順序は符号なしバイト順。
// ビューは木を共有する個体が少なくとも一つ生きている間に使う。
