// SPDX-License-Identifier: CC0-1.0 AND LicenseRef-SplitMix64-Public-Domain
// 同梱するGenericHash::mixはSebastiano VignaのSplitMix64。原著者のPD提供声明と無条件許諾に従う。
// 原典: https://prng.di.unimi.it/splitmix64.c
// 提供条件: https://github.com/reinsirk/sirkit-cpp-library/blob/main/LICENSES/LicenseRef-SplitMix64-Public-Domain.txt
// 参考（種類数制約）: https://atcoder.jp/contests/typical90/tasks/typical90_ah
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subarray_frequency_windows.md
//
// API一覧
// SubarrayFrequencyBounds: maximum_distinctは種類数、maximum_occurrencesは各値の出現回数の上限。既定は無制限。
// analyze_frequency_bounded_subarrays(values,bounds): 両上限を満たす空でない連続部分列の数と最長区間を求める。
// SubarrayFrequencyResult: countと0始まりの半開区間[left,right)。最長が複数なら左端最小、なければ[0,0)。
// length() const: 最長区間の長さ。
// 値は64ビット以下の整数。期待O(n+1)時間、O(d+1)領域。dは処理中の最大種類数。
