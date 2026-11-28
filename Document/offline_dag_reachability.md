# DAGの到達可能性クエリ

有向非巡回グラフで、始点から終点へ進めるかを複数まとめて調べます。1回の走査で最大64種類の端点を処理します。[本体](../Graph/Offline_DAG_Reachability.cpp)

## `offline_dag_reachability`

```text
std::vector<bool> offline_dag_reachability(
    int n, std::span<const std::pair<int, int>> edges,
    std::span<const std::pair<int, int>> queries);
```

頂点番号は `[0,n)`。辺と照会はどちらも `{始点, 終点}` の組で渡します。返り値は照会順で、始点と終点が同じ場合は `true` です。平行辺があるグラフも扱えます。通常版では、有向閉路があると `std::invalid_argument` を送出します。

頂点数 $n$、辺数 $m$、照会数 $q$ に対し、$O((n+m)(1+\lceil p/64\rceil)+q)$ 時間、$O(n+m+q)$ 領域です。$p$ は、同じ頂点への照会とトポロジカル順に逆らう照会を除いたときの、異なる始点数と異なる終点数の小さいほうです。

## 使用例

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 3}};
const std::array queries{std::pair{0, 3}, std::pair{1, 2}, std::pair{3, 0}};
const auto reachable = sirkit::offline_dag_reachability(4, edges, queries);
// reachable == {true, false, false}
```

[使用例](../Examples/offline_dag_reachability_example.cpp)・[AtCoder Verify](../Verify/current/submissions/offline_dag_reachability.cpp)
