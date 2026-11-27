// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_distance_frequency.md
//
// API一覧
// tree_distance_frequency(graph): 長さnのvector<int64_t>を返す。d番目は距離dの異なる非順序頂点対の個数。
// graphは0始まりの非重み単純無向木。距離は辺数、添字0は常に0、空入力は空の結果を返す。
// 頂点数は2^23以下。重心分解と整数畳み込みでO(n log²(n+1))時間・O(n)領域。
