// SPDX-License-Identifier: CC0-1.0
// 参考（静的な弦の交差照会）: https://atcoder.jp/contests/abc405/tasks/abc405_f
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/chord_crossing_queries.md
//
// API一覧
// count_chord_crossings(chords,queries): 各照会弦と端点が円周上で交互に現れる登録弦の個数。照会順にintで返す。
// 弦は円周順の符号付き64ビット端点の組。両端は順不同。登録の重複は別々に数える。
// 端点を共有する弦・同じ点を結ぶ弦はいずれも交差数に含めない。
// 登録数M<=INT_MAX、照会数Q。O(1+(M+Q)*log(M+Q+1))時間・O(M+Q+1)領域。
