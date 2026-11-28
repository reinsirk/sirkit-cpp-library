# 最小費用 b-flow

各頂点の供給・需要と、各辺の流量の上下限を指定し、総費用を最小化します。費用が負の閉路や、出入りが相殺される自己辺も扱えます。[本体](../Graph/Min_Cost_B_Flow.cpp)

## `min_cost_b_flow`

```text
std::optional<BFlowResult> min_cost_b_flow(
    std::span<const std::int64_t> supply, std::span<const BFlowEdge> edges);
```

`supply[v]` は頂点 `v` から出る流量の合計から入る流量の合計を引いた値です。頂点番号は `supply` の添字で、正の値は供給、負の値は需要を表します。`BFlowEdge{from, to, lower, upper, cost}` は `from` から `to` への有向辺で、`lower`・`upper` は流量の両端、`cost` は流量単位あたりの費用です。`lower` と `cost` に負の値を使えます。

制約を満たす流量がなければ `std::nullopt` を返します。解がある場合、`BFlowResult::flow` は入力順の流量、`BFlowResult::potential` は頂点ごとの双対ポテンシャルです。

残余辺の換算費用 `cost + potential[from] - potential[to]` は非負です。逆向きの残余辺では費用の符号を反転します。総費用は必要な桁数の型で `cost * flow` を合計してください。

頂点数 $n$、辺数 $m$、費用の最大絶対値 $C$ に対し、時間は $O((n+1)^2(n+m+1)(1+\log(1+nC)))$、領域は $O(n+m+1)$ です。最大流で実現可能な流量を探し、費用スケーリングで改善します。同梱の `mf_graph` の使い方は[最大流](max_flow.md)を参照してください。

通常版は範囲外の端点や逆転した上下限に `std::invalid_argument` を投げます。残余容量、供給の合計、スケーリングした費用やポテンシャルの計算が `std::int64_t` の範囲を超える場合は `std::overflow_error` になります。

## 使用例

```cpp
const std::array<std::int64_t, 2> supply{1, -1};
const std::array edges{sirkit::BFlowEdge{0, 1, 0, 2, 1},
                       sirkit::BFlowEdge{1, 0, 0, 1, -3}};
const auto result = sirkit::min_cost_b_flow(supply, edges);
// 最適流量は順に2,1。総費用は2-3=-1。
```

[使用例](../Examples/min_cost_b_flow_example.cpp)・[Library Checker Verify](../Verify/current/submissions/min_cost_b_flow.cpp)
