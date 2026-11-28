// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lcm_spanning_tree.md
//
// API一覧
// LcmSpanningTreeEdge{from,to,weight}: 頂点番号は0始まり、辺の重みはuint64_t。
// minimum_lcm_spanning_tree(labels): 正のuint32_tラベルを持つ完全グラフの最小全域木の辺を返す。
// 辺の重みは両端のラベルの最小公倍数。空と頂点1個は空の辺列。ラベル0はinvalid_argument。
// n<=INT_MAX。全ラベルを最大公約数で割った最大値Aに対しO(n log(U+1)+A log²(A+1))時間、
// O(n+A log(A+1))領域。Uは最大ラベル。最小ラベルが他を割り切る場合はO(n log(U+1))時間。
