# 最小直径全域木

木の中で最も離れた頂点対の距離を直径と呼びます。非負重みの無向グラフから、その直径が最小の全域木を求めます。

```cpp
using Edge = sirkit::MinimumDiameterTreeEdge;
const std::array edges{Edge{0, 1, 1}, Edge{1, 2, 2}, Edge{0, 2, 9}};
const auto tree = sirkit::minimum_diameter_spanning_tree(3, edges);
// tree->diameter == 3。tree->edge_ids に辺番号 0 と 1 が入る。
```

## `minimum_diameter_spanning_tree`

```text
std::optional<MinimumDiameterTreeResult> minimum_diameter_spanning_tree(
    int n, std::span<const MinimumDiameterTreeEdge> edges);
```

端点は `[0,n)` です。戻り値の `diameter` は最小直径、`edge_ids` は選んだ辺の0始まりの入力添字です。多重辺と重み0を扱え、自己辺は無視します。空または非連結なら `std::nullopt`、1頂点なら直径0です。最適直径が `std::int64_t` の範囲を超えると `std::overflow_error` を送出します。

頂点のほか、辺の途中も中心の候補に含めます。全頂点までの最長距離を最小にする点から、最短路木を構成します。$O(n^2+nm\log(n+1))$ 時間・$O(n^2+m)$ 領域です。

[使用例](../Examples/minimum_diameter_spanning_tree_example.cpp) ／ [Verify](../Verify/current/submissions/minimum_diameter_spanning_tree.cpp) ／ [Hassin・Tamir (1995)](https://doi.org/10.1016/0020-0190(94)00183-Y)
