// SPDX-License-Identifier: CC0-1.0 AND LicenseRef-SplitMix64-Public-Domain
// 同梱するGenericHash::mixはSebastiano VignaのSplitMix64。原著者のPD提供声明と無条件許諾に従う。
// 原典: https://prng.di.unimi.it/splitmix64.c
// 提供条件: https://github.com/reinsirk/sirkit-cpp-library/blob/main/LICENSES/LicenseRef-SplitMix64-Public-Domain.txt
// 参考（3文字の出現回数比較）: https://atcoder.jp/contests/abc455/tasks/abc455_e
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/three_symbol_frequency_counts.md
//
// API一覧
// analyze_three_symbol_frequencies(text,symbols={'A','B','C'}): 3種類の文字の出現回数を空でない全区間で比較する。
// ThreeSymbolFrequencyCounts: all_equalは3回数が等しい区間、all_distinctは3回数が相異なる区間の個数。
// equal_pairsはsymbolsの組(0,1),(0,2),(1,2)ごとの同頻度区間数。3つとも等しい区間も各組に含む。
// symbolsは相異なるバイト。他の文字は指定3文字の出現数に入らない。n<INT32_MAX。
// 期待O(n+1)時間・O(n+1)領域。
