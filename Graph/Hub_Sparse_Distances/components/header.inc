// SPDX-License-Identifier: CC0-1.0
// 参考（ハブ経由の移動と少数の短縮辺を使うVerify）: https://yukicoder.me/problems/no/3810
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/hub_sparse_distances.md
//
// API一覧
// HubSparseDistances(gates, shortcuts): 任意の異なるu,vをgates[u]+gates[v]で結ぶ無向グラフに追加辺を与えて前処理する。
// Shortcut{from,to,cost}: 0-indexedの無向辺。多重辺、自己辺、費用0を許す。gates[i]はUINT64_MAX/2以下。
// distance(u,v): 最短距離をuint64_tで返す。同じ頂点の距離は0。O(1)。size(): 頂点数。O(1)。
// 追加辺m本、頂点n個。0辺を縮約しハブ経由以上の辺を除いた残りの端点数をkとする。
// 前処理はO(1+(n+m)α(n+1)+k^3)時間、O(1+n+k²)領域。nはINT_MAX以下。
