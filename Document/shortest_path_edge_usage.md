# 最短路で使う辺

始点から終点への最短路を考え、各辺を通る最短路があるか、すべての最短路がその辺を通るかを調べます。[本体](../Graph/Shortest_Path_Edge_Usage.cpp)

## `shortest_path_edge_usage`

```text
ShortestPathEdgeUsage shortest_path_edge_usage(
    int n, std::span<const ShortestPathEdge> edges,
    int source, int target, bool undirected = false);
```

`ShortestPathEdge{from, to, weight}` の頂点番号は $0$ 始まりで、重みは正の `std::uint64_t` です。無向グラフには `undirected = true` を指定し、各辺を一度だけ渡します。

- `distance` は最短距離です。到達不能、または距離が `std::uint64_t` の上限を超える場合は `std::nullopt` を返します。
- `edge_use[i]` は入力の辺 $i$ の判定です。通る最短路がなければ `none`、一部の最短路が通れば `some`、すべてが通れば `all` です。`ShortestPathEdgeUse` で表します。

各最短路を始点からの距離に沿って見ると、辺の重みが正なので距離は増え続けます。最短路で使える辺がある距離をまたぐとき、ほかにその距離をまたぐ辺がなければ、その辺はすべての最短路に含まれます。逆にすべての最短路に含まれる辺では、その辺の両端の距離の間をまたげる辺がほかにありません。

頂点数 $n$、辺数 $m$ に対し $O((n+m)\log(n+m+1))$ 時間、$O(n+m)$ 領域です。

## 使用例

```cpp
const std::array edges{sirkit::ShortestPathEdge{0, 1, 2},
                       sirkit::ShortestPathEdge{1, 2, 3},
                       sirkit::ShortestPathEdge{0, 2, 5}};
const auto routes = sirkit::shortest_path_edge_usage(3, edges, 0, 2);
// 距離は5。どの辺も一部の最短路だけが通るので、すべてsome。
```

[使用例](../Examples/shortest_path_edge_usage_example.cpp)・[Verify](../Verify/current/submissions/shortest_path_edge_usage.cpp)
