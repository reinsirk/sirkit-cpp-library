# 最短ハミルトン路・巡回

全頂点を1度ずつ通る最小コストの経路を求めます。有向辺、負のコスト、辺がない頂点対も扱えます。

| API | 動作 |
|---|---|
| `shortest_hamiltonian_path<Weight>(n, start, cost_at, end = std::nullopt)` | 始点を固定した経路。終点は省略できる。 |
| `shortest_hamiltonian_cycle<Weight>(n, start, cost_at)` | 全頂点を通った後、始点に戻る巡回路。 |

戻り値は `{cost, vertices}` で、解がなければ `std::nullopt` です。巡回路の `vertices` には始点を末尾にも入れます。頂点1個の場合は移動なしでコスト0です。

`cost_at(u,v)` は辺コストか `std::optional<Weight>` を返します。空の `optional` は辺なしを表します。頂点番号は0始まりです。比較と加算が定数時間なら $O(n^2 2^n)$ 時間、$O(n 2^n)$ 領域です。`Weight` の既定は `std::int64_t` です。途中の和も型に収まることが前提です。範囲外の頂点番号は `std::out_of_range`、部分集合やDP表の大きさを表せなければ `std::length_error` です。

[使用例](../Examples/hamiltonian_route_example.cpp) ／ [AtCoder Verify](../Verify/current/submissions/hamiltonian_route_abc180_e.cpp) ／ [AOJ Verify](../Verify/current/submissions/hamiltonian_route_aoj_dpl_2_a.cpp)
