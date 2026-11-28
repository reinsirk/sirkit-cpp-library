# 差分制約の実現可能性と範囲

`DifferenceConstraint{from, to, upper}` は `x[to] - x[from] <= upper` を表します。`solve_difference_constraints(n, constraints, anchor)` に渡すと、全制約を満たす整数解があるかを調べます。添字は 0 始まりで、基準点 `anchor` は省略すると 0 です。

- `feasible` が真なら解が存在します。`values` は `values[anchor] = 0` を満たす解の一例です。
- `bounds[v].lower` と `bounds[v].upper` はそれぞれ基準点からの差 `x[v] - x[anchor]` の最小値と最大値です。非有界な側は `std::nullopt` になります。

例えば `x[1]-x[0] <= 4` と `x[0]-x[1] <= -2` があれば、頂点 1 の範囲は 2 以上 4 以下です。制約の重みは絶対値 `10^9` 以下です。第 4 引数の `inf` は Bellman–Ford が扱う有限距離の範囲を決めます。既定値は `2^60` で、有限距離を `(-inf, inf)` に収めます。計算量は `O(n(n+m))` 時間・`O(n+m)` 領域です。

[使用例](../Examples/difference_constraints_example.cpp) · [Verify問題](https://atcoder.jp/contests/past21-open/tasks/past21_o)
