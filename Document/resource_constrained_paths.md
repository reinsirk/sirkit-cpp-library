# 資源制約付き最短路

移動に資源を使い、途中で補充できる有向グラフの最短時間を求めます。残量が多く所要時間が短い状態があれば、同じ頂点でそれより不利な状態を省きます。

`ResourceConstrainedPaths(n, edges, source, capacity, initial)` に `ResourcePathArc{from, to, spend, gain, time}` を渡します。辺を通るには資源 `spend` が必要です。支払った後に `gain` を受け取り、補充と初期値は `capacity` で止まります。頂点番号は 0 始まりで、資源量と時間は `uint64_t` で、時間がその範囲に収まる経路を扱います。

- `minimum_time(v, required=0)` は残量を `required` 以上にして到着する最短時間を返します。条件を満たせなければ `nullopt` です。
- `labels(v)` は他の組に支配されない `{remaining, time}` を残量順に参照できます。

上限を C、辺数を m とすると、構築は最悪 `O(n + (m+1)(C+1) log((m+1)(C+1)+2))` 時間、`O((n+m)(C+1))` 領域です。最短時間の照会は頂点の組数を k として `O(log(k+2))` です。

[使用例](../Examples/resource_constrained_paths_example.cpp) · [Verify問題](https://atcoder.jp/contests/abc164/tasks/abc164_e)
