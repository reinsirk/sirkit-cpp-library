# 頂点重みを更新できる木の距離和

`TreeDistanceSums<T>(n, edges, weights={}, root=0)` で、頂点重みを持つ木を準備します。辺は `TreeDistanceSumEdge<T>{from, to, length}`、頂点番号は 0 始まりです。辺長の既定値は 1 で、空の頂点重み列を渡すと全頂点を 0 で初期化します。

- `add(v, delta)` は頂点 `v` の重みを加算します。
- `sum_at(v)` は全頂点について「頂点重み × `v` までの距離」の和を返します。
- `sum_path(u, v)` は `sum_at(x)` をパス上の全頂点 `x` について足し合わせます。両端も含みます。

加減乗算ができる `T` を使います。組み込み整数の計算は型幅で周回し、剰余型を使えばその法での結果を得られます。

構築は `O(n)` 時間・領域、加算と照会は `O(log²(n+1))` 時間です。

[使用例](../Examples/tree_distance_sums_example.cpp) · [Verify問題](https://atcoder.jp/contests/scpc2026-div1/tasks/scpc2026_div1_l)
