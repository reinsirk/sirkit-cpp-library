// SPDX-License-Identifier: CC0-1.0
// 参考（和・長さの条件）: https://atcoder.jp/contests/awc0198/tasks/awc0198_d
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subarray_sum_counts.md
//
// API一覧
// SubarraySumBounds: minimum_sum,maximum_sumは和の閉区間の端。nulloptで片側を無制限にする。
// minimum_length,maximum_lengthは長さの閉区間。既定では空でないすべての連続部分列を対象にする。
// count_subarrays_by_sum(values,bounds): 和・長さが両条件を満たす連続部分列を数える。
// 全要素が非負ならO(n+1)時間。負の要素を含む場合はO((n+1)log(n+2))時間。領域はO(n+1)。
// 0<minimum_length<=maximum_length。列長はINT_MAX未満、累積和はint64_tに収まること。
