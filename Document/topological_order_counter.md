# トポロジカル順序の数え上げ

有向辺 `u → v` ごとに `u` が先に来る頂点順序を数えます。頂点数が 20 以下のグラフで使えます。

`TopologicalOrderCounter(n, edges)` に頂点数と、0 始まりの端点の組を渡します。`count()` は順序の総数を `uint64_t` で返します。閉路があれば 0 通りです。頂点がなければ空の順序が 1 通りあります。

`order_at(k)` で辞書順の第 `k` 番目の順序を取り出せます。`rank(order)` は順序の辞書順順位を返します。順位は 0 始まりです。範囲外の順位や無効な順序には `nullopt` が返ります。

```cpp
const std::vector<std::pair<int, int>> edges{{1, 0}, {1, 2}};
const sirkit::TopologicalOrderCounter orders(3, edges);
assert(orders.count() == 2);
assert(orders.order_at(0).value() == std::vector({1, 0, 2}));
```

前計算は `O(n·2^n + m)` 時間・`O(2^n + n)` 領域です。個数の取得は `O(1)`、順位と順序の変換は `O(n²)` です。

[使用例](../Examples/topological_order_counter_example.cpp) · [Verify](https://atcoder.jp/contests/abc041/tasks/abc041_d) · [トポロジカルソート](topological.md)
