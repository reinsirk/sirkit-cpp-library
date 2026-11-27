// SPDX-License-Identifier: CC0-1.0
// 参考（Link-Cut Tree）: https://doi.org/10.1016/0022-0000(83)90006-5
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/incremental_minimum_spanning_forest.md
//
// API一覧
// IncrementalMinimumSpanningForest(n): n頂点0辺の無向グラフ。頂点番号は[0,n)。O(n+1)時間・領域。
// add_edge(u,v,weight): 符号付き64ビット重みの辺をオンラインで追加する。償却O(log(n+1))。
// IncrementalMsfInsertion: edge_idは0始まりの入力辺番号、selectedは新しい辺の採否。
// removed_edgeは置換した既存の辺番号、置換しない場合はnullopt。自己辺は不採用、多重辺可、同重みの辺は置換しない。
// total_weight(): 森の重み合計。int64_t範囲外ではoverflow_error。以後も辺を追加できる。
// size()/edge_count()/component_count(): 頂点数/追加済み辺数/現在の連結成分数。各O(1)。合計の照会もO(1)。
// 森の辺に対応する枠だけを使い回す。頂点数は2n−1をintで表現できる範囲（n=0も可）。
