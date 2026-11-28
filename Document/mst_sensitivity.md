# 最小全域森の交換辺

最小全域森から辺を1本外したり、指定した辺を必ず使ったりするとき、どの辺と入れ替えれば重みが最小になるかを求めます。[本体](../Graph/MST_Sensitivity.cpp)

## `minimum_spanning_forest_sensitivity`

```text
SpanningForestSensitivity minimum_spanning_forest_sensitivity(
    int n, std::span<const SpanningTreeEdge> edges);
```

$0$ 始まりの頂点番号を持つ無向グラフを渡します。`SpanningTreeEdge{from, to, weight}` の重みは符号付き64ビット。非連結グラフ、多重辺、負の重みも扱えます。

- `forest` は元のグラフの最小全域森に含まれる辺番号の列です。
- `exchange[e]` は、`e` が森に含まれるならその辺を外す際に追加する辺、含まれないならその辺を強制する際に外す辺です。
- 交換が不可能なら `std::nullopt`。森の橋を外したり、自己辺を強制したりするときが該当します。同じ重みの交換候補では辺番号の小さいものを選びます。

辺番号は入力順の添字で、重みの合計は呼び出し側で計算します。頂点数 $n$、辺数 $m$ に対し $O(n\log(n+1)+m(\log(m+1)+\log(n+1)))$ 時間、$O(n\log(n+1)+m)$ 領域です。

## 使用例

```cpp
const std::array edges{sirkit::SpanningTreeEdge{0, 1, 1},
                       sirkit::SpanningTreeEdge{1, 2, 2},
                       sirkit::SpanningTreeEdge{0, 2, 3}};
const auto cuts = sirkit::minimum_spanning_forest_sensitivity(3, edges);
// forest == {0,1}。辺0を外すと辺2を追加し、辺2を強制すると辺1を外す。
```

[使用例](../Examples/mst_sensitivity_example.cpp)・[除外のVerify](../Verify/current/submissions/mst_sensitivity_delete.cpp)・[強制のVerify](../Verify/current/submissions/mst_sensitivity_force.cpp)
