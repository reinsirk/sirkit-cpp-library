// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/disjoint_subarray_max_sum.md
// 参考（パス上のマッチングへの帰着）: https://info.atcoder.jp/entry/algorithm_lectures/matching_on_path
//
// API一覧
// DisjointSubarrayMaxSum<Score=int64_t>(values): 交わらない空でない部分配列を選ぶ最大和を準備する。
// n要素をO(n log(n+1))時間・O(n)領域で前処理する。隣接する区間も選べる。途中の加減算はScoreに収まること。
// size(): 元の要素数。O(1)。
// max_sum(k): ちょうどk区間を選んだときの最大和。k>nならnullopt。O(1)。
// intervals(k): 最大和を達成する0始まり半開区間を左から並べる。k>nならnullopt。O(n)時間・領域。
