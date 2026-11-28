// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/binary_range_statistics.md
//
// API一覧
// BinaryRangeStatistics(bits=""): '0'と'1'の列から構築。BinaryRangeStatistics(n): 0をn個。O(n)時間・領域。
// flip(l,r): [l,r)の0と1を反転。assign(l,r,bit): 区間を指定ビットにする。O(log(n+1))。
// stats(l,r): length,onesと、0/1それぞれのprefix/suffix/longestを返す。空区間は各項目0。O(log(n+1))。
// all_stats(): 全体統計、size(): 要素数。O(1)。添字は0始まり、bitはbool。コピーは独立。
