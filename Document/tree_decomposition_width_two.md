# 木分解（幅2以下）

無向グラフを、頂点を最大3個ずつまとめた「袋」の木にします。各辺の両端を含む袋があり、各頂点を含む袋は木の中でつながります。

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 0}};
const auto result = sirkit::tree_decomposition_width_two(3, edges);
// result->bags に袋、result->tree_edges に袋を結ぶ木の辺が入る。
```

## `tree_decomposition_width_two`

```text
std::optional<WidthTwoTreeDecomposition> tree_decomposition_width_two(
    int n, std::span<const std::pair<int, int>> edges);
```

`bags` に袋の一覧、`tree_edges` に袋番号で表した木の辺を返します。袋の頂点は `bag.vertices` の先頭 `bag.size` 個に入り、頂点番号と袋番号は0始まりです。`width` は最大袋サイズから1を引いた値です。

木幅が3以上なら `std::nullopt` を返します。自己辺と多重辺を扱え、非連結でも袋は1本の木になります。空グラフでは袋と木の辺が空で、幅は0です。平均 $O(n+m)$ 時間、$O(n+m)$ 領域です。

[使用例](../Examples/tree_decomposition_width_two_example.cpp) ／ [Verify](../Verify/current/submissions/tree_decomposition_width_2.cpp)
