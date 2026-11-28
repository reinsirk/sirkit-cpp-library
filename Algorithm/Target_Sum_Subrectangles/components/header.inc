// SPDX-License-Identifier: CC0-1.0 AND LicenseRef-SplitMix64-Public-Domain
// 同梱するGenericHash::mixはSebastiano VignaのSplitMix64。原著者のPD提供声明と無条件許諾に従う。
// 原典: https://prng.di.unimi.it/splitmix64.c
// 提供条件: https://github.com/reinsirk/sirkit-cpp-library/blob/main/LICENSES/LicenseRef-SplitMix64-Public-Domain.txt
// 参考（釣り合うマスの個数）: https://atcoder.jp/contests/abc410/tasks/abc410_f
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/target_sum_subrectangles.md
//
// API一覧
// count_subrectangles_with_sum(height,width,target,value_at): 合計がtargetの空でない長方形の個数(uint64_t)。
// value_at(row,col)は0始まりの値(int64_t)。符号付きの帯・累積和はint64_t、個数はuint64_tに収まること。
// H,Wは非負。空なら0。O(1+min(H,W)^2*max(H,W))期待時間、O(1+max(H,W))領域。
