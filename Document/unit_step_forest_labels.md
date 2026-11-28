# 隣接頂点の差が 1 になる整数割り当て

無向森の各頂点に整数を割り当てます。各頂点には許される閉区間があり、辺で結ばれた 2 頂点の値の差の絶対値を 1 にします。

`unit_step_forest_labels(n, edges, allowed)` に、0 始まりの辺と頂点順の `IntegerLabelInterval{lower, upper}` を渡してください。値を `x` に固定するには `{x, x}`、制約がなければ既定値を使います。

条件を満たせるなら頂点順の `int64_t` 列、満たせなければ `nullopt` を返します。時間と領域はともに `O(n)` です。

[使用例](../Examples/unit_step_forest_labels_example.cpp) · [Verify問題](https://atcoder.jp/contests/arc063/tasks/arc063_c)
