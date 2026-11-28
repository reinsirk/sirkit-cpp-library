// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/permutation_order.md
//
// API一覧
// permutation_lehmer_code(p): p[i]より小さく、iより後ろにある要素の個数を各桁として返す。
// permutation_from_lehmer_code(code): 各桁0<=code[i]<n-iから順列を復元する。
// permutation_rank(p): 0始まりの辞書順の順位。uint64_tに収まらなければnullopt。
// permutation_unrank(n,rank): 長さnで指定順位の順列。rank>=n!ならnullopt。
// 順列は0..n-1を一度ずつ含む。空順列の順位は0。各O(n log(n+1)+1)時間・O(n+1)領域。
