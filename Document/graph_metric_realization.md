# 最短距離表からグラフを復元

距離表と同じ全頂点間の最短距離を持ち、辺の重みの合計が最小のグラフを求めます。頂点は距離表の行・列と一致します。

`realize_graph_metric(n, distance, directed=false)` に、0 始まりの `distance(u, v)` を渡します。距離は 64 ビット以下の整数または `optional<uint64_t>` で、到達不能は `nullopt`。同じ頂点間は 0、異なる頂点間の有限距離は正です。無向グラフでは距離が対称である必要があります。

戻り値は `edges` と `total_weight` です。矛盾があれば戻り値は `nullopt`、合計が `uint64_t` の範囲を超える場合は `total_weight` だけが `nullopt` になります。

中継頂点を 1 つ挟んでも距離が等しい辺は省けます。距離の取得が `O(1)` なら、復元は `O(n³)` 時間・`O(n²)` 領域です。

[使用例](../Examples/graph_metric_realization_example.cpp) · [Verify問題](https://atcoder.jp/contests/abc074/tasks/arc083_b)
