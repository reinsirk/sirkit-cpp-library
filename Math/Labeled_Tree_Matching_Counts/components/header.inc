// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/labeled_tree_matching_counts.md
// 数学的参考: https://info.atcoder.jp/entry/algorithm_lectures/undirected_vertex_geography
//
// API一覧
// labeled_tree_exposable_root_counts(limit): 指定頂点を未対応にする最大マッチングが存在するラベル付き木の個数。
// labeled_tree_perfect_matching_counts(limit): 完全マッチングを持つラベル付き木の個数。
// 戻り値のn番目が頂点数n、法998244353。0頂点は0。上限はFormalPowerSeries::max_precision()。
// 順にO(limit log(limit+1))時間、どちらもO(limit+1)領域。
