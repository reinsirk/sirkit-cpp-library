// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/hamming_distance_matching.md
//
// API一覧
// hamming_distances(text,pattern): 各開始位置の不一致バイト数を返す。
// approximate_match_positions(text,pattern,max_mismatches): 上限以下になる開始位置を昇順で返す。
// 空パターンは全境界に不一致0で一致。textより長いパターンは空の結果。
// 非空パターンを照合できる場合n+m-1<=2^23。L=bit_ceil(n+m-1)、文字cの個数をa_c,b_cとする。
// 頻度の低い文字は位置の組、頻度の高い文字はNTTで数える。各O(n+m+sum min(a_c*b_c,L log L))時間、O(n+m+1)領域。
