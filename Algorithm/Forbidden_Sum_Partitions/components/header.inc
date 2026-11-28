// SPDX-License-Identifier: CC0-1.0 AND LicenseRef-SplitMix64-Public-Domain
// 同梱するGenericHash::mixはSebastiano VignaのSplitMix64。原著者のPD提供声明と無条件許諾に従う。
// 原典: https://prng.di.unimi.it/splitmix64.c
// 提供条件: https://github.com/reinsirk/sirkit-cpp-library/blob/main/LICENSES/LicenseRef-SplitMix64-Public-Domain.txt
// 参考（禁止和を避ける分割）: https://atcoder.jp/contests/abc370/tasks/abc370_e
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/forbidden_sum_partitions.md
//
// API一覧
// count_partitions_avoiding_sums<Count>(values,forbidden): int64_t列を空でない区間に分け、どの区間和も禁止値に含まれない分割数。
// forbiddenは単一値または列。順不同・重複可。空のvaluesは1。Countの加減算で数え、累積和はint64_tに収める。
// nを長さ、kを禁止値の指定数とし、型の演算がO(1)なら期待O(1+n*(k+1)+k log(k+1))時間、O(n+k+1)領域。
