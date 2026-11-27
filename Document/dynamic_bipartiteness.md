# オフライン動的二部性

無向グラフに辺を追加・削除し、それぞれの照会時点でグラフ全体を2色に塗り分けられるか判定します。

```cpp
sirkit::DynamicBipartiteness graph(3);
graph.add_edge(0, 1);
graph.add_edge(1, 2);
graph.query_bipartite();
const auto diagonal = graph.add_edge(0, 2);
graph.query_bipartite();
graph.remove_edge(diagonal);
graph.query_bipartite();
const auto answers = graph.solve();  // {true, false, true}
```

## `DynamicBipartiteness`

```text
explicit DynamicBipartiteness(int n = 0);
```

辺のない $n$ 頂点のグラフを $O(1)$ で作ります。頂点番号は `[0,n)` です。

## `add_edge`

```text
std::size_t add_edge(int u, int v);
```

無向辺を追加し、0始まりの辺 ID を償却 $O(1)$ で返します。自己ループや多重辺も追加できます。

## `remove_edge`

```text
void remove_edge(std::size_t id);
```

このグラフで発行した有効な ID を渡し、対応する1本だけを $O(1)$ で削除します。

## `query_bipartite`

```text
std::size_t query_bipartite();
```

その時点の照会を記録して、0始まりの回答添字を $O(1)$ で返します。照会はそれまでに呼び出した追加・削除を順に反映します。自己ループが有効な照会の答えは `false` です。

## `solve`

```text
std::vector<bool> solve() const;
```

記録した照会の答えを照会順に返します。操作を追記して再実行できます。頂点数 $n$、追加辺数 $e$、照会数 $q$ に対し、$O(n+q+e\log(q+1)\log(n+2))$ 時間、$O(n+q+e\log(q+1))$ 追加領域です。追加辺数は $2^{32}-1$ 以下で、時刻区間を展開する配列が確保できることが前提です。

## `size`

```text
int size() const noexcept;
```

頂点数を $O(1)$ で返します。

## `edge_count`

```text
std::size_t edge_count() const noexcept;
```

削除済みも含む辺の追加件数を $O(1)$ で返します。

## `query_count`

```text
std::size_t query_count() const noexcept;
```

記録した照会数を $O(1)$ で返します。

[オフライン動的連結性](offline_dynamic_connectivity.md) の照会対象は連結成分の情報です。[Rollback Weighted Union-Find](rollback_weighted_union_find.md) は重み差の制約を追加し、最後の追加から順に巻き戻せます。動的二部性では辺の有効期間を区間に分け、巻き戻し付きUnion-Findで色の偶奇を管理します。

[使用例](../Examples/dynamic_bipartiteness_example.cpp) ／ [BFSとの参照テスト](../Verify/current/tests/dynamic_bipartiteness_reference_test.cpp)
