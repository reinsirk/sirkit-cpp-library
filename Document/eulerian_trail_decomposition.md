# オイラー路分解

グラフの全辺を、同じ辺を二度通らない小道に分けます。小道の本数が最小となる分解を返します。小道の中で同じ頂点を何度通っても構いません。

`minimum_eulerian_trail_decomposition<Directed>(n, edges)` に頂点数と辺の端点の列を渡します。頂点は 0 始まりで、辺の ID は入力順です。既定は無向グラフです。有向グラフは `<true>` を指定します。自己辺と多重辺も扱えます。

返り値は小道の列です。各小道の `vertices` は通る頂点、`edges` は対応する辺 ID です。辺のない頂点は結果に含めません。辺が 0 本なら空列が返ります。

```cpp
const std::vector<std::pair<int, int>> edges{{0, 1}, {0, 2}, {0, 3}};
const auto trails = sirkit::minimum_eulerian_trail_decomposition<true>(4, edges);
assert(trails.size() == 3);
```

時間・領域はともに `O(n + m)` です。仮の頂点を一つ追加し、小道の始点や終点になる頂点とつなぎます。得られたオイラー閉路を仮の頂点で切ると、最小本数の小道に分かれます。

[使用例](../Examples/eulerian_trail_decomposition_example.cpp) · [有向版の Verify](https://atcoder.jp/contests/arc220/tasks/arc220_b) · [単一のオイラー路](eulerian_trail.md)
