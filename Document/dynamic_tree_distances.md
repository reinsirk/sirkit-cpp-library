# 辺重みを変更できる木の距離

形が固定の木や森で、辺重みを更新しながらパス上の重みを合計します。[本体](../Graph/Dynamic_Tree_Distances.cpp)

- `DynamicTreeDistances<T>(n, edges, root = 0)` に `DynamicTreeDistanceEdge<T>{from, to, weight}` を渡します。頂点と辺番号は $0$ 始まりで、辺番号は入力順です。指定した根と別の成分では、その成分で最小番号の頂点を根にします。
- `set_weight(id, w)` は辺重みを上書きし、`weight(id)` は現在の値を返します。
- `root_distance(v)` は成分の根から、`distance(u, v)` は2頂点間の重みの総和を返します。別成分なら `distance` は `std::nullopt` です。

型 `T` は既定で `int64_t` です。計算は整数型のビット幅で周回します。辺重みの差分を子側の部分木に加えるため、更新と距離照会は $O(\log(n+1))$ です。構築と領域は $B=\lceil n/32\rceil$ として $O(n+B\log(B+1))$ です。

## 使用例

```cpp
const std::array edges{sirkit::DynamicTreeDistanceEdge<>{0, 1, 4},
                       sirkit::DynamicTreeDistanceEdge<>{1, 2, 6}};
sirkit::DynamicTreeDistances<> tree(3, edges);
tree.set_weight(1, 8);
const auto total = tree.distance(0, 2);  // 4 + 8 = 12
```

[使用例](../Examples/dynamic_tree_distances_example.cpp)・[Verify](../Verify/current/submissions/dynamic_tree_distances.cpp)
