# 辺を追加できる全点対最短距離

辺を追加したり重みを減らしたりしながら、任意の2頂点の最短距離を求めます。[本体](../Graph/Incremental_APSP.cpp)

- `IncrementalAPSP(n, edges = {}, undirected = false)` は $0$ 始まりのグラフを受け取ります。辺は `IncrementalAPSPEdge{from, to, weight}` で、重みは非負の `uint32_t` です。`undirected = true` で初期辺と後述の追加辺を両方向に扱います。
- `add_edge(u, v, w)` で辺を追加します。すでに距離が `w` 以下なら無視します。重みの増加や辺の削除がある問題では、操作を逆順に処理できるか検討してください。
- `distance(u, v)` は最短距離を `int64_t` で返します。到達不能なら `std::nullopt` です。照会は $O(1)$、追加は $O(n^2)$ です。

初期辺がある場合はワーシャル–フロイド法で $O(n^3+m)$、初期辺がなければ $O(n^2)$ で構築します。保持領域は $O(n^2)$ です。

## 使用例

```cpp
const std::array edges{sirkit::IncrementalAPSPEdge{0, 1, 4}};
sirkit::IncrementalAPSP paths(3, edges);
paths.add_edge(1, 2, 8);
const auto first = paths.distance(0, 2);  // 12
```

[使用例](../Examples/incremental_apsp_example.cpp)・[Verify](../Verify/current/submissions/incremental_apsp.cpp)
