# DSU on Tree

すべての頂点について、部分木に含まれる頂点を集計します。色の種類数や最頻値のように、頂点の追加・削除で集計を更新できるときに使います。

`DsuOnTree(parents)` に 0 始まりの親配列を渡してください。根の親は `-1` です。構築済みの `SubtreeIntervals` も渡せます。

`run(add, remove, answer)` は頂点 `u` を追加・削除するときに `add(u)`、`remove(u)` を呼びます。`answer(v)` が呼ばれたとき、集計に入っているのは `v` の部分木の頂点すべてです。集計を空にして呼び出すと、完了時にも空に戻ります。

構築は `O(n)` 時間・領域です。各コールバックが `O(1)` なら `run` は `O(n log(n + 1))` 時間、追加領域 `O(1)` で実行できます。

[使用例](../Examples/dsu_on_tree_example.cpp) · [部分木の最頻値の Verify](https://atcoder.jp/contests/abc454/tasks/abc454_g)
