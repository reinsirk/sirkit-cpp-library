# 最短歩道の個数

非負重みのグラフで、始点から各頂点への最短歩道を数えます。異なる辺を使った歩道は別に数えます。[本体](../Graph/Shortest_Walk_Counts.cpp)

- `count_shortest_walks<Count>(n, edges, source, undirected = false)` に `ShortestPathEdge{from, to, weight}` を渡します。頂点は $0$ 始まり、重みは `uint64_t` です。始点での空の歩道も含めます。
- `distance[v]` は最短距離です。到達不能、または距離が `uint64_t` に収まらなければ `std::nullopt` です。`count[v]` は有限の場合の個数で、無限なら `infinite[v]` が真になります。未到達や無限の個数欄は $0$ です。
- 個数の型は既定で `uint64_t` で、$2^{64}$ で周回します。剰余を取りたい場合は、`0` と `1` から作れて `+=` を持つ剰余型を指定できます。

重みがすべて $1$ なら幅優先探索で $O(n+m)$ 時間、それ以外はダイクストラ法と最短距離を保つ辺のトポロジカル処理で $O((n+m)\log(n+m+1))$ 時間です。領域は $O(n+m)$ です。

## 使用例

```cpp
const std::array edges{sirkit::ShortestPathEdge{0, 1, 1}, sirkit::ShortestPathEdge{0, 2, 1},
                       sirkit::ShortestPathEdge{1, 3, 1}, sirkit::ShortestPathEdge{2, 3, 1}};
const auto walks = sirkit::count_shortest_walks(4, edges, 0);
// walks.distance[3] は 2、walks.count[3] は 2。
```

[使用例](../Examples/shortest_walk_counts_example.cpp)・[Verify](../Verify/current/submissions/shortest_walk_counts.cpp)
