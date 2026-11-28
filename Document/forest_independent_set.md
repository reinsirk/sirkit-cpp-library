# 森の独立集合

`ForestIndependentSet(n, edges)` に 0 始まりの無向森を渡します。独立集合は同じ辺の両端を同時に選ばない頂点集合です。

- `count<T>()` は空集合も含めて個数を返します。法付きの型 `T` を指定すると、その法での余りを得られます。
- `count_by_vertex<T>()` は各頂点を選ばない場合と選ぶ場合の個数を、頂点番号順に `{excluded, included}` で返します。
- `maximum_weight<T>(weights)` は頂点重みの和が最大になる集合とその和を `{weight, vertices}` で返します。負の重みも扱い、空集合も候補です。

数え上げの引数に、各頂点の `{excluded, included}` の重みを渡すこともできます。頂点を選んだかどうかに応じた重みを掛け、独立集合ごとの積を足します。どちらの数え上げにも除算は不要です。

構築と各操作は `O(n+1)` 時間・領域です。

[使用例](../Examples/forest_independent_set_example.cpp) · [Verify問題](https://atcoder.jp/contests/dp/tasks/dp_p)
