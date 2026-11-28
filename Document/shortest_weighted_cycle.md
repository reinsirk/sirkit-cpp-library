# 重み付きグラフの最小閉路

有向・無向グラフから、通る辺の重みの和が最小の単純閉路を1つ求めます。[本体](../Graph/Shortest_Weighted_Cycle.cpp)

```text
shortest_weighted_cycle(n, edges, directed = true, through = std::nullopt);
```

- 辺は `WeightedCycleEdge{from, to, weight}` です。頂点番号は $0$ 始まりで、重みは `std::uint64_t`、省略時は $1$ です。自己辺と多重辺を扱えます。
- 無向グラフなら `directed = false` を指定します。頂点 `through` を指定すると、その頂点を含む閉路だけを調べます。
- 結果は重みの合計 `weight` と `cycle` を持ちます。`cycle.vertices[i]` から次の頂点へ進む辺が `cycle.edge_ids[i]` です。辺番号は入力順です。
- 閉路が存在しない場合や、その最小の重みの和が `std::uint64_t` の上限を超える場合は `std::nullopt` を返します。

頂点数 $n$、辺数 $m$ のとき、頂点を指定すれば $O((n+m)\log(n+m+1))$ 時間で求められます。指定しない場合は各頂点から調べるため、最悪でその $n$ 倍かかります。領域は $O(n+m+1)$ です。

## 使用例

```cpp
using Edge = sirkit::WeightedCycleEdge;
const std::array edges{Edge{0, 1, 3}, Edge{1, 2, 1}, Edge{2, 0, 2}};
const auto cycle = sirkit::shortest_weighted_cycle(3, edges);
// weight == 6。0 → 1 → 2 → 0。
```

[使用例](../Examples/shortest_weighted_cycle_example.cpp)・[重み付きのVerify](../Verify/current/submissions/shortest_weighted_cycle.cpp)・[指定頂点のVerify](../Verify/current/submissions/shortest_weighted_cycle_through.cpp)
