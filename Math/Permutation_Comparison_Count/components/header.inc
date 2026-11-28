// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/permutation_comparison_count.md
// 参考（下降集合の包除原理）: https://arxiv.org/abs/math/0507224
//
// API一覧
// count_permutations_by_comparisons<Prime=998244353>(comparisons): 長さnの順列の個数を法Primeで返す。
// n=comparisons.size()+1。各文字は隣接する要素の大小: '<', '>', '?'（どちらでもよい）。
// 空の文字列はn=1。文字が範囲外ならinvalid_argument。n<min(Prime,2^22)が必要。
// 最悪O(n log²(n+1))時間・O(n)領域。Primeはuint32_tの素数。値の入力は不要。
