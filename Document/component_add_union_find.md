# 成分全体に加算できる Union-Find

連結成分を結合しながら、現在の成分の全頂点へ同じ値を加えます。成分を結合しても各頂点の値はそのままです。

`ComponentAddUnionFind<T>(n)` は値 0 の単独成分を `n` 個作ります。初期値の配列を渡すこともできます。頂点は 0 始まりで、値の既定の型は `int64_t` です。

- `unite(u, v)` は成分を結合します。すでに同じ成分なら `false` を返します。
- `add_component(v, delta)` は `v` の成分の全頂点に `delta` を加えます。
- `get(v)` は頂点の値、`component_sum(v)` は成分の値の合計を返します。
- `root(v)`、`issame(u, v)`、`size(v)` は代表頂点、同成分かどうか、成分の頂点数を返します。

構築は `O(n)` 時間・領域、上記の各操作は償却 `O(α(n))` です。加算と合計は型の範囲を超えると型幅で周回します。

[使用例](../Examples/component_add_union_find_example.cpp) · [Verify問題](https://yukicoder.me/problems/no/1054)
