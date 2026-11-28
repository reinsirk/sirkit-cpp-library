# 有効辺の端点積を集計

候補の無向辺を有効・無効に切り替えながら、有効辺ごとの両端の頂点値の積を集計します。

`DynamicEdgeProducts<T>(n, edges, values={})` に候補辺と初期値を渡します。頂点は 0 始まりで、辺番号は入力位置です。候補は単純グラフで、最初は全辺が無効です。初期値を省くと全頂点が 0 になります。

- `set_active(id, enabled)` で辺の状態を設定し、`active(id)` で確認できます。
- `add(v, delta)` で頂点値を加算します。`value(v)` は頂点値、`neighbor_sum(v)` は有効辺で隣接する頂点の値の和です。
- `sum_edge_products()` は有効辺の端点積の合計を返します。

候補辺数を m として、準備は `O(n+m log(m+1))` 時間・`O(n+m)` 領域です。頂点加算と隣接和は `O(1+√m)` 時間、ほかの操作は `O(1)` 時間です。整数計算は型幅で周回します。

[使用例](../Examples/dynamic_edge_products_example.cpp) · [Verify問題](https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=3509&lang=ja)
