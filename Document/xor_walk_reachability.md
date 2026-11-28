# 有向グラフの XOR 歩道

指定した始点から、通った辺の重みの XOR を調べます。同じ頂点や辺を何度でも通れ、始点では辺を通らない歩道も数えます。[本体](../Graph/Xor_Walk_Reachability.cpp)

- `XorWalkReachability(n, edges, source, directed = true)` は頂点と XOR の組を探索します。頂点は 0 始まり、辺は `XorWalkEdge{from, to, weight}` です。`directed = false` で各辺を両向きにたどれます。
- `reachable(v, x)` は XOR が `x` となる歩道があるかを返します。`min_xor(v)` と `max_xor(v)` は最小値と最大値を返し、到達不能の場合は `std::nullopt` です。いずれも $O(1)$ です。

最大の辺重みのビット幅を $B$ とすると、構築は $O((n+m)2^B)$ 時間、$O(n2^B+m)$ 領域です。たとえば重みが 10 ビットなら、頂点ごとに最大 1024 状態を調べます。[大きな重みを扱う無向版](undirected_xor_walks.md)

## 使用例

```cpp
const std::array edges{sirkit::XorWalkEdge{0, 1, 7}, sirkit::XorWalkEdge{1, 1, 6}};
const sirkit::XorWalkReachability walks(3, edges, 0);
const auto minimum = walks.min_xor(1);  // 7 XOR 6 = 1
```

[使用例](../Examples/xor_walk_reachability_example.cpp)・[Verify](../Verify/current/submissions/xor_walk_reachability.cpp)
