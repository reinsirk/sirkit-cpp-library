# 頂点彩色数

20頂点以下の無向グラフで、辺の両端を異なる色にするために必要な最小色数を求めます。色の割り当ても復元できます。

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 0}};
const auto color_count = sirkit::chromatic_number(3, edges); // 3
const auto colored = sirkit::minimum_vertex_coloring(3, edges);
// colored->color_count == 3。colored->colors[v] に頂点 v の色が入る。
```

## `chromatic_number`

```text
std::optional<int> chromatic_number(int n, std::span<const std::pair<int, int>> edges);
```

最小色数を返します。頂点数は0〜20、端点の番号は `[0,n)` です。多重辺を扱えます。自己辺があれば `std::nullopt`、空グラフなら0を返します。頂点数 $n$、辺数 $m$ に対し、$O((1+n^3\log(n+1))2^n+m)$ 時間、$O(2^n)$ 補助領域です。

## `minimum_vertex_coloring`

```text
std::optional<MinimumVertexColoring> minimum_vertex_coloring(
    int n, std::span<const std::pair<int, int>> edges);
```

`color_count` に最小色数、`colors[v]` に頂点 `v` の0始まりの色番号を返します。入力条件と自己辺の扱いは `chromatic_number` と共通で、$O((1+n^4)2^n+m)$ 時間、$O(2^n)$ 補助領域です。

[使用例](../Examples/chromatic_number_example.cpp) ／ [Verify](../Verify/current/submissions/chromatic_number.cpp) ／ [参考論文](https://eccc.weizmann.ac.il/report/2006/044/)
