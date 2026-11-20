# 負辺を含む単一始点最短路

[標準版](../Graph/Bellman-Ford.cpp)・[SPFA版](../Graph/Bellman-Ford_SFPA.cpp)・[使用例](../Examples/bellman_ford_example.cpp)

負辺を含む有向グラフで、1つの始点から全頂点への最短距離を求めます。
`WGraph`は`std::vector<std::vector<Edge>>`、辺は`Edge{int to, std::int64_t w}`です。
両ファイルは単独でも、同時に貼り付けても利用できます。

## API

| 呼び出し | 結果 |
| --- | --- |
| `BellmanFord(graph, distances, start, inf=kInf)` | 全距離を上書きし、到達可能な負閉路があれば`true` |
| `SPFA(graph, distances, start, inf=kInf)` | 同じ結果を、更新頂点を再処理する方式で計算 |

`inf`は正の`std::int64_t`で、省略時は`kInf = std::int64_t{1} << 60`です。
未到達は`inf`、負閉路を経由して到達できる頂点は`-inf`になります。
未到達成分の負閉路では`true`になりません。負閉路から戻れない始点や、別の枝の距離は有限のままです。

```cpp
WGraph graph(4);
graph[0].emplace_back(1, -2);
graph[1].emplace_back(2, 5);
std::vector<std::int64_t> distance;
const std::int64_t inf = 1'000'000'000'000;
const bool negative = BellmanFord(graph, distance, 0, inf);
// negative=false, distance={0,-2,3,inf}
// SPFA(graph, distance, 0, inf)も同じ結果。
```

## 前提・例外

辺重みは`[-1e9, 1e9]`、頂点数は`INT_MAX`以下、始点と辺の行先は有効な頂点番号とします。
負閉路の影響を受けない有限の最短距離は`(-inf, inf)`に収まる必要があります。
これを外れる場合は`overflow_error`、`inf<=0`は`invalid_argument`です。
途中の経路の値が範囲を外れても、最終的な有限距離だけが判定対象です。
入力のグラフは変更しません。呼び出しごとに内部状態を所有します。

既定では始点と、計算中に参照する辺の終点・重みを検査し、不正なら`out_of_range`です。
未到達成分を検査のためだけに走査しません。容量超過は`length_error`です。
検査有効時は例外になっても既存の距離配列を保ちます。
`SKIP_RUNTIME_CHECKS`で入力検査を省く場合は、例外時の出力は未規定です。
容量・算術範囲・`inf`の正値の保護は検査省略時も残ります。

## 計算量と使い分け

V=頂点数、E=辺数とし、どちらも最悪O(VE+V+E)時間、O(V)追加領域です。
`BellmanFord`は辺の緩和を反復し、`SPFA`は更新された頂点を待ち行列で再処理します。
SPFAも最悪時間が改善するとは限りません。

方式・検証の詳細は[記録](../Audit/graph-document-details-2026-11-15.md)を参照してください。
