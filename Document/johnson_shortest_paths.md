# Johnson 法による全点対最短路

負の重みを含む疎な有向グラフで、全点対の最短距離を求めます。各始点からの距離を 1 行ずつ受け取れるので、合計だけ必要なら全行を保持せずに使えます。

`for_each_johnson_distance_row(n, edges, on_row)` に頂点数、辺の列、行を受け取る関数を渡します。辺は `JohnsonEdge{from, to, weight}` です。頂点は 0 始まりで、重みの絶対値は `INT64_MAX / (4 * max(1, n))` 以下です。

`on_row(source, row)` に始点番号と、頂点順の距離の `span` が渡ります。距離は `int64_t` で、未到達は `nullopt` です。行を後から使う場合はコピーしてください。負閉路があれば `false` が返り、行は渡されません。処理できれば `true` が返ります。

辺数を m とすると、コールバックを除き `O(n(n + m log(m + 1)))` 時間、`O(n + m)` 領域です。

[使用例](../Examples/johnson_shortest_paths_example.cpp) · [行合計の Verify](https://yukicoder.me/problems/no/1344) · [負閉路と距離行列の Verify](https://onlinejudge.u-aizu.ac.jp/problems/GRL_1_C) · [Johnson の論文](https://doi.org/10.1145/321992.321993)
