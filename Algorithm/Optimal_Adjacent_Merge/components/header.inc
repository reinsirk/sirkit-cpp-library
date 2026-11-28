// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/optimal_adjacent_merge.md
// 参考（Garsia–Wachs法）: https://doi.org/10.1017/S0956796819000194
//
// API一覧
// optimal_adjacent_merge(weights): 非負uint64_t列を隣接併合で1要素にする最小費用と、併合順を返す。
// 併合費用は併合する2要素の重みの和。新しい要素の重みも同じ和。期待O(n log(n+1))時間、O(n)領域。
// [AdjacentMergeResult] cost: 最小費用。uint64_tを超えるとoverflow_error。
// merges[i]: i回目に併合する左右のID。初期IDは0..n-1、i回目にできるIDはn+i。
// 空列・1要素なら費用0、併合なし。nはintに収まること。
