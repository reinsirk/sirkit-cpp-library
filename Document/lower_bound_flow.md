# 下限付きフロー

辺の流量に下限と上限を指定して、条件を満たす循環流や、始点から終点へ流せる量の最小値・最大値を求めます。[本体](../Graph/Lower_Bound_Flow.cpp)

| 操作 | 意味 |
| --- | --- |
| `LowerBoundFlow<Cap>(n)` | 頂点番号 $0$ から $n-1$ の有向グラフを用意。容量型の既定値は `std::int64_t` |
| `add_edge(u, v, lower, upper)` | 下限と上限のある辺を追加し、その辺番号を返す |
| `circulation()` | 全頂点で流入量と流出量が等しい流れを、入力順の辺流量で返す |
| `min_flow(s, t, limit)`、`max_flow(s, t, limit)` | 区間 $[0,\mathrm{limit}]$ で最小・最大の流量 `value` と、入力順の辺流量 `edge_flow` を返す |
| `size()`、`edge_count()` | 頂点数、辺数 |

容量は $0\leq\mathrm{lower}\leq\mathrm{upper}$ とし、自己辺と多重辺を扱えます。自己辺を除いた各頂点の流入・流出下限の和と、不足を補う総流量が `Cap` に収まることが必要です。

始点と終点には異なる頂点を指定します。`limit` の既定値は `Cap` の最大値です。条件を満たす流れがない場合は `std::nullopt` を返すので、辺を保持したまま条件を変えて呼べます。

辺追加は償却 $O(1)$。解を求めるには高々2回の最大流と $O(n+m)$ の追加処理が必要です。領域は $O(n+m)$ です。

## 使用例

```cpp
sirkit::LowerBoundFlow<int> graph(3);
graph.add_edge(0, 1, 2, 4);
graph.add_edge(1, 2, 1, 3);
// 0→2のmin_flowは2、max_flowは3。
```

[使用例](../Examples/lower_bound_flow_example.cpp)・[Verify](../Verify/current/submissions/lower_bound_flow.cpp)
