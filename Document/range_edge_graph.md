# 区間に辺を張るグラフ

ある区間の各頂点から、別の区間の全頂点へ辺を張ります。補助頂点を使って辺を減らし、元の頂点間の最短距離を保ちます。[本体](../Graph/Range_Edge_Graph.cpp)

## `RangeEdgeGraph`

```text
explicit RangeEdgeGraph(int n);
```

非負の頂点数を指定し、元の頂点は $0$ から $n-1$ で番号を付けます。最初の層の出発費用と到着費用はどの頂点も $0$ です。$O(n+1)$ 時間・領域です。

## `add_layer`

```text
std::size_t add_layer(std::span<const std::int64_t> departure,
                      std::span<const std::int64_t> arrival);
```

出発費用と到着費用をそれぞれ $n$ 個渡します。戻り値は新しい層の番号です。入力の列は保持しません。どちらの費用も、同じ層内の最大値と最小値の差を `std::int64_t` の非負範囲に収めてください。追加時間は償却 $O(n+1)$ で、層ごとに $O(n+1)$ 領域を使います。

## `add_range_edge`

```text
void add_range_edge(int l, int r, int left, int right, std::int64_t w,
                    std::size_t layer = 0);
```

半開区間 `[l,r)` の各頂点 `u` から `[left,right)` の各頂点 `v` へ、重み `w + departure[u] + arrival[v]` の有向辺をまとめて張ります。片方が空区間なら何も追加しません。補助頂点を2個、辺を $O(\log(n+1))$ 本追加します。追加時間は償却 $O(\log(n+1))$ です。

最小の追加辺の重みを `std::int64_t` に収めてください。ダイクストラ法を使う場合は、この重みが非負になるように指定し、距離の計算には値域を満たす型を使います。

## `add_edge`

```text
void add_edge(int u, int v, std::int64_t w);
```

元の頂点 `u` から `v` へ重み `w` の辺を1本張ります。層の費用は加えません。償却 $O(1)$ 時間です。

## `size`

```text
int size() const noexcept;
```

元の頂点数を $O(1)$ で返します。

## `vertex_count`

```text
int vertex_count() const noexcept;
```

補助頂点を含む頂点数を $O(1)$ で返します。

## `edges`

```text
const std::vector<WeightedEdge> &edges() const noexcept;
```

拡張したグラフの辺列への参照を $O(1)$ で返します。`WeightedEdge` は `std::tuple<int, int, std::int64_t>` で、始点、終点、重みの順です。追加操作で辺のイテレータは無効になることがあります。

## `graph`

```text
WGraph graph() const;
```

拡張したグラフを隣接リストにして返します。拡張後の頂点数 $V$、辺数 $E$ に対し $O(V+E)$ 時間・領域です。

## 使用例

```cpp
sirkit::RangeEdgeGraph graph(8);
graph.add_range_edge(0, 4, 4, 8, 3);  // 頂点0..3から頂点4..7へ重み3
graph.add_edge(7, 0, 2);
auto adjacent = graph.graph();
```

[使用例](../Examples/range_edge_graph_example.cpp)・[乗車区間と降車区間のVerify](../Verify/current/submissions/range_edge_graph_express.cpp)
