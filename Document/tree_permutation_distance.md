# 木の移動距離を最大にする順列

全頂点を別々の行き先に割り当て、各頂点から行き先までの距離の和を最大化します。

`TreePermutationDistance(n, edges)` に `TreePermutationEdge{from, to, weight}` を渡します。頂点番号は 0 始まりで、辺の重みは正の `uint64_t`、既定値は 1 です。

- `maximum()` は最大距離を返します。値が `uint64_t` の範囲を超える場合は `nullopt` です。
- `permutation()` は最大値を達成する行き先の列を 1 つ返します。
- `optimal_count(modulus=1000000007)` は最大値を達成する順列数を正の法で割った余りです。

構築と順列の出力は `O(n)` 時間、最大値の取得は `O(1)` 時間です。順列数は重心を除いた各枝を使った包除で `O(n²)` 時間、`O(n)` 追加領域で求めます。

[使用例](../Examples/tree_permutation_distance_example.cpp) · [Verify問題](https://atcoder.jp/contests/arc087/tasks/arc087_d)
