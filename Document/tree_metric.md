# 距離行列から重み付き木を復元

全頂点間の距離から、正の重みを持つ無向木を復元します。入力で番号を付けた頂点が木の全頂点です。

`reconstruct_tree_metric(n, distance)` の `distance(u, v)` に 0 始まりの頂点間距離を返す関数を渡します。距離は 64 ビット以下の整数で、同じ頂点では 0、異なる頂点では正かつ対称である必要があります。

戻り値は `TreeMetricEdge{from, to, weight}` の列です。距離に一致する木がなければ `nullopt` になります。

最小全域木を候補にして、すべての距離が一致するかを調べます。距離の取得が `O(1)` なら、復元は `O(n²)` 時間・`O(n)` 追加領域です。

[使用例](../Examples/tree_metric_example.cpp) · [Verify問題](https://atcoder.jp/contests/abc451/tasks/abc451_e)
