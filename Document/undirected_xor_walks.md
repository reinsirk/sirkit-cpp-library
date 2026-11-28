# 無向グラフの最小・最大 XOR 歩道

無向グラフで、通った辺の重みを XOR した値を調べます。同じ頂点や辺を何度通っても構いません。[本体](../Graph/Undirected_Xor_Walks.cpp)

- `UndirectedXorWalks(n, edges)` は 0 始まりの頂点と 64 ビット符号なしの重みを受け取ります。辺は `XorWalkEdge{from, to, weight}` です。構築は $O(64(n+m))$ 時間、$O(n+m+64c)$ 領域です。$c$ は XOR が 0 でない閉じた歩道を持つ連結成分の数です。
- `min_xor(u, v)` は最小値を $O(1)$、`max_xor(u, v)` は最大値を $O(64)$ で返します。頂点間を行き来できなければ `std::nullopt` です。
- `attainable(u, v, x)` は XOR が `x` となる歩道があるかを $O(64)$ で調べます。`count_pairs_at_most(x)` は最小値が `x` 以下である異なる頂点の組を数えます。計算量は $O(n(\log(n+1)+64))$ 時間、追加 $O(n)$ 領域です。

全域森を基準に閉じた歩道の XOR 基底を作ります。基底の各ピボットを消した頂点ラベルを XOR すると、頂点間の最小値になります。[有向グラフの少ビット版](xor_walk_reachability.md)

## 使用例

```cpp
const std::array edges{sirkit::XorWalkEdge{0, 1, 6}, sirkit::XorWalkEdge{1, 2, 7},
                       sirkit::XorWalkEdge{0, 2, 4}};
const sirkit::UndirectedXorWalks walks(4, edges);
const auto minimum = walks.min_xor(0, 2);  // 1
const auto absent = walks.min_xor(0, 3);   // std::nullopt
```

[使用例](../Examples/undirected_xor_walks_example.cpp)・[Verify](../Verify/current/submissions/undirected_xor_walks.cpp)
