# 木のいもす法

固定の無向森で、パスへの加算をまとめて記録します。各頂点や各辺の最終値をまとめて取得したいときに使います。

`TreePathDifference<T>(n, edges, root=0)` を作ります。頂点は 0 始まりで、各無向辺を `{u, v}` で 1 回渡してください。初期値はすべて 0、型 `T` の既定値は `int64_t` です。加減算は型幅で周回します。

- `add_vertex_path(u, v, delta)` は両端を含むパス上の各頂点に `delta` を加えます。
- `add_edge_path(u, v, delta)` はパス上の各辺に `delta` を加えます。
- `vertex_values()` と `edge_values()` はそれぞれ頂点番号順と入力辺順に累計を返します。

頂点と辺の累計は別々です。両端が別成分のときは加算せず `false` を返します。累計を取得したあとも加算できます。

加算は 1 回 `O(1)`、累計の取得は `O(n)` 時間と追加領域を使います。構築時間と領域は `O(n + B log(B + 1))`、`B = ceil(n / 32)` です。

[使用例](../Examples/tree_path_difference_example.cpp) · [Verify問題](https://atcoder.jp/contests/awc0100/tasks/awc0100_j)
