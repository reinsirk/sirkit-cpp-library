// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/adjacent_swap_parity.md
// 参考（同値の要素の対応と交換回数の偶奇）: https://atcoder.jp/contests/arc214/editorial/15190
//
// API一覧
// adjacent_swap_distances_by_parity(from,to,compare={}): 隣接交換で一致させる最小回数を偶数・奇数別に返す。
// AdjacentSwapParityDistances{even,odd}: それぞれoptional<uint64_t>。その偶奇で到達できないときnullopt。
// 同値な2要素の交換も1回と数える。比較・列の要件はadjacent_swap_distanceと同じ、長さn<=INT_MAX。
// 計算量はO((n+1)log(n+2))時間・O(n+1)領域。
