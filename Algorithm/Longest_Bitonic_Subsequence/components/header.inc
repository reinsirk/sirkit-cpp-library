// SPDX-License-Identifier: CC0-1.0
// 参考（山型部分列）: https://atcoder.jp/contests/typical90/tasks/typical90_bh
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/longest_bitonic_subsequence.md
//
// API一覧
// BitonicSubsequenceResult{indices,peak}: 最長部分列の元の0始まり添字と、その添字列内での頂点の位置。
// longest_bitonic_subsequence<Strict=true>(values): 増加してから減少する最長部分列を復元。
// Strict=falseなら等値可。片側だけでもよい。空列には空のindicesとpeak=0。O(1+n log(n+1))時間・O(n+1)領域。
// 値はコピー構築・代入可能でoperator<が狭義弱順序。浮動小数のNaNは検査有効時に拒否。
