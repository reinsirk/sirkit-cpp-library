# トポロジカル順序の列挙

辺が指定する先後関係を満たす頂点順序を、最大 K 個求めます。頂点数が多く、必要な候補が少ないときに使います。

`enumerate_topological_orders(n, edges, k)` に頂点数、0 始まりの有向辺の列、欲しい順序の上限を渡します。存在する順序が K 個未満なら、その個数だけ返ります。順序は相異なり、返り値に並ぶ順番は未規定です。閉路があれば空列、頂点が 0 個なら空の順序が 1 個あります。

```cpp
const std::vector<std::pair<int, int>> edges{{0, 2}};
auto orders = sirkit::enumerate_topological_orders(3, edges, 2);
assert(orders.size() == 2);
```

辺数を m、実際に返した順序数を r とすると、`O((r + 1)(n + m))` 時間、`O(n + m + rn)` 領域です。再帰を使わず、入次数が 0 の候補から次の頂点を選びます。

[使用例](../Examples/enumerate_topological_orders_example.cpp) · [Verify](https://atcoder.jp/contests/typical90/tasks/typical90_bs) · [順序の数え上げ](topological_order_counter.md)
