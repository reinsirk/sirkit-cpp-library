# 森で頂点部分集合の連結成分数を数える

`forest_subset_components<T>(n, edges)` は、頂点をいくつか選んだときの連結成分数を集計します。両端の頂点を選んだ辺だけで連結性を判定します。

`edges` は 0 始まりの無向辺です。戻り値の `counts[k]` が成分数 `k` の選び方の個数で、`counts[0]` が空集合です。

頂点ごとに選択の重みを付けたい場合は、`ForestSubsetWeight<T>{omit, take}` の列を第 3 引数に渡します。各頂点で選んだ係数の積を選び方ごとに足します。

子の部分木と結ぶ際に、親子の両方を選んだ場合だけ成分数を 1 減らします。計算量は `O((n+1)²)` 時間・`O(n+1)` 領域です。

[使用例](../Examples/forest_subset_components_example.cpp) · [Verify問題](https://atcoder.jp/contests/abc287/tasks/abc287_f)
