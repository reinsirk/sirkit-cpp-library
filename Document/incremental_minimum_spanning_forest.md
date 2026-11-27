# 辺追加と最小全域森

重み付き無向辺を1本ずつ追加し、その時点の最小全域森を保ちます。辺の追加順を先に決める必要はありません。

```cpp
sirkit::IncrementalMinimumSpanningForest forest(3);
forest.add_edge(0, 1, 5);
forest.add_edge(1, 2, 8);
const auto update = forest.add_edge(0, 2, 1);
// update.edge_id == 2, update.selected == true, update.removed_edge == 1
// forest.total_weight() == 6
```

## `IncrementalMinimumSpanningForest`

```text
explicit IncrementalMinimumSpanningForest(int n);
```

辺のない $n$ 頂点の無向グラフを作ります。頂点番号は `[0,n)` で、$2n-1$ が `int` に収まる範囲です。初期化は $O(n+1)$ 時間・領域です。

## `add_edge`

```text
IncrementalMsfInsertion add_edge(int from, int to, std::int64_t weight);
```

結果の `edge_id` は0始まりの入力辺番号、`selected` は採否、`removed_edge` は置換した辺番号です。置換がなければ `removed_edge` は `std::nullopt` です。自己辺は採用せず、多重辺は個別に扱います。同重みの場合は既存の辺を保ちます。償却 $O(\log(n+1))$ 時間です。

## `total_weight`

```text
std::int64_t total_weight() const;
```

森の重み合計を $O(1)$ で返します。`std::int64_t` の範囲外なら `std::overflow_error` を送出し、その後も辺を追加できます。

## `size`

```text
int size() const noexcept;
```

頂点数を $O(1)$ で返します。

## `edge_count`

```text
std::size_t edge_count() const noexcept;
```

不採用の辺も含む追加件数を $O(1)$ で返します。

## `component_count`

```text
int component_count() const noexcept;
```

現在の連結成分数を $O(1)$ で返します。

`LinkCutTree` の公開 API を同梱しています。 [Link-Cut Tree](link_cut_tree.md)

[使用例](../Examples/incremental_minimum_spanning_forest_example.cpp) ／ [Verify](../Verify/current/submissions/incremental_minimum_spanning_forest.cpp)
