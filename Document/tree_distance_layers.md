# 木で指定距離にある頂点を探す

`TreeDistanceLayers(n, edges)` は無向森を受け取り、距離が指定された頂点を1つ探します。距離はパスの辺数で、端点の番号は $0$ 始まりです。[本体](../Graph/Tree_Distance_Layers.cpp)

- `vertex_at_distance(v, k)` は `v` から距離 $k$ の頂点を返します。存在しなければ `-1` です。$O(\log(n+1))$ 時間です。
- `farthest(v)` は同じ成分の最遠頂点の1つ、`eccentricity(v)` はそこまでの距離です。`diameter_endpoints(v)` は成分の直径の両端を返します。いずれも $O(1)$ です。

どの頂点でも、直径の両端のうち少なくとも一方が最遠頂点です。その端へ向かうパスから $k$ 番目の頂点を取ります。構築と領域は $B=\lceil n/32\rceil$ として $O(n+B\log(B+1))$ です。

## 使用例

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 3}};
const sirkit::TreeDistanceLayers tree(4, edges);
const int vertex = tree.vertex_at_distance(1, 2);  // 3
```

[使用例](../Examples/tree_distance_layers_example.cpp)・[Verify](../Verify/current/submissions/tree_distance_layers.cpp)
