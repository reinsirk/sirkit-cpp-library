// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/multiset_permutation.md
//
// API一覧
// MultisetPermutation<T>(values): 重複を含む列を多重集合として保持する。辞書順はTの大小順。
// size(): 元の要素数をO(1)で返す。count(): 異なる順列の総数をBigInteger<>で返す。空列でも1。
// rank(values): 0始まりの辞書順順位を返す。要素や個数が一致しなければnullopt。
// unrank(index): BigInteger<>の0始まりの順位から順列を復元する。範囲外ならnullopt。
// n要素、異なる値s種類、総数の桁数Dに対し、構築O(n(log(n+1)+D))、rank/unrankはO(n(D+log(s+1)))。
// 構築時の追加領域O(n+D)、保持O(s+D)。countのコピーO(D)、rank/unrankの追加O(s+D)（結果を除く）。
// nはintの上限以下。超過はlength_error。Tの比較はO(1)とする。
