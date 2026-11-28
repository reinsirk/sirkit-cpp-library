# 部分木の辺加算とパス和

固定の無向森で、部分木の全辺に同じ値を加えながら、2 頂点を結ぶパスの辺重みの総和を求めます。辺重みの既定の型は `int64_t` で、負の値を扱えます。

`SubtreeEdgeAddDistances<T>(n, edges, root=0)` に `Edge{from, to, weight}` を渡します。頂点は 0 始まりです。指定した根と別の成分では、最小番号の頂点が根になります。

- `add_subtree(v, delta)` は `v` の部分木内の全辺に `delta` を加えます。親から `v` への辺は範囲外です。
- `root_distance(v)` は成分の根から `v` までのパス和を返します。
- `distance(u, v)` は `u` から `v` までのパス和を返します。連結していなければ `nullopt` です。

加算と照会は `O(log(n + 1))` です。構築時間と領域は `O(n + B log(B + 1))`、`B = ceil(n / 32)` です。演算結果が型の範囲を超えると型幅で周回します。

[使用例](../Examples/subtree_edge_add_distances_example.cpp) · [Verify問題](https://yukicoder.me/problems/no/900)
