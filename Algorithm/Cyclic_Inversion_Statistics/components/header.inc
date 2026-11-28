// SPDX-License-Identifier: CC0-1.0
// 参考（巡回シフトの転倒数の差分）: https://atcoder.jp/contests/arc218/editorial/19876
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/cyclic_inversion_statistics.md
//
// API一覧
// CyclicInversionStatistics(values,less={}): 列の比較順位から構築。O(n*log(n+1)+1)時間・O(n+1)領域。
// valuesは長さが分かるランダムアクセス範囲。lessは既定でstd::less<>。同値の組は転倒に含まない。
// inversion_count(): 現在の転倒数。rotation_inversion_sum(): 全n個の左巡回シフトの転倒数の和。各O(1)。
// left_rotation_counts(): 0回からn-1回の左巡回シフトの転倒数を順に返す。O(n+1)時間・領域。
// swap_adjacent(i): 0始まりの位置iとi+1を交換。O(1)。末尾と先頭の交換は扱わない。
// size()/empty(): 長さ/空か。O(1)。集計値はint64_t。n<=2642246、空構築可、ムーブ元は空。
