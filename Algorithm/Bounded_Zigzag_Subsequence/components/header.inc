// SPDX-License-Identifier: CC0-1.0
// 参考（直前の変化方向を持つDP）: https://atcoder.jp/contests/awc0207/editorial/28440
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bounded_zigzag_subsequence.md
//
// API一覧
// bounded_zigzag_subsequence(values,weights,min_step,max_step): 変化の向きが交互で、隣り合う選択値の差の絶対値が
// [min_step,max_step]に入る部分列のうち、重み和が最大のものを返す。O(n*log(n+1)+1)時間・O(n+1)領域。
// BoundedZigzagSelection{weight,indices}: 最大重み和と、選んだ0始まりの添字列。空を選ぶと0。
// 1<=min_step<=max_step、値と重みはint64_t、差の上限はuint64_t。最大和がint64_tを超えるとoverflow_error。
