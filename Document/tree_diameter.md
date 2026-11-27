# 木の直径

非負の重み付き無向木で、距離が最も長いパスとその長さを求めます。

```cpp
const std::vector<sirkit::TreeDiameterEdge> edges{{0, 1, 4}, {1, 2, 3}, {1, 3, 8}};
const auto result = sirkit::tree_diameter(4, edges);
// result.length == 12。result.path は {0, 1, 3} またはその逆順。
```

## `tree_diameter`

```text
TreeDiameterResult tree_diameter(int n, std::span<const TreeDiameterEdge> edges);
```

`TreeDiameterEdge{from, to, weight}` は0始まりの端点を持つ無向辺です。各辺を1回ずつ渡します。重みは非負の `std::int64_t` です。

戻り値の `length` は直径の長さ、`path` は経路上の頂点列です。頂点が0個なら長さ0と空の列を、1個の場合や全辺の重みが0の場合は長さ0と `{0}` を返します。直径の長さが `std::int64_t` の範囲を超えたときの例外は `std::overflow_error` です。

頂点数を `n` として、時間と追加領域は `O(n)` です。走査と経路の復元は反復で行います。

[使用例](../Examples/tree_diameter_example.cpp) ／ [Verify](../Verify/current/submissions/tree_diameter.cpp) ／ [Luzhiledの解説](https://ei1333.github.io/luzhiled/snippets/tree/tree-diameter.html)
