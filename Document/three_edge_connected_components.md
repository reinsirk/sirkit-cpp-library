# 3辺連結成分

無向グラフから辺を2本まで取り除いても、互いに到達できる頂点を同じ成分にまとめます。

```cpp
const std::array edges{std::pair{0, 1}, std::pair{0, 1}, std::pair{0, 1}};
const auto result = sirkit::three_edge_connected_component_ids(3, edges);
// result.count == 2。result.ids[0] == result.ids[1]。
// sirkit::three_edge_connected_components(3, edges) なら {{0, 1}, {2}}。
```

## `three_edge_connected_component_ids`

```text
ThreeEdgeComponentIds three_edge_connected_component_ids(
    int n, std::span<const std::pair<int, int>> edges);
```

`count` は成分数、`ids[v]` は頂点 `v` の成分番号です。頂点番号と成分番号は0始まりです。平行辺は別々に数え、自己辺は無視します。空グラフの成分数は0です。頂点数 $n$、辺数 $m$ に対して $O((n+m)\log(n+1))$ 時間、$O(n+m)$ 領域です。

## `three_edge_connected_components`

```text
std::vector<std::vector<int>> three_edge_connected_components(
    int n, std::span<const std::pair<int, int>> edges);
```

成分ごとの頂点一覧を返します。各一覧の頂点番号は昇順です。入力条件と計算量は `three_edge_connected_component_ids` と共通です。

[使用例](../Examples/three_edge_connected_components_example.cpp) ／ [Verify](../Verify/current/submissions/three_edge_connected_components.cpp) ／ [参考論文](https://doi.org/10.4230/LIPIcs.SEA.2021.20)
