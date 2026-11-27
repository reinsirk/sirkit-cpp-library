# 最小全域木・最小全域森

無向グラフの各連結成分を、重みの合計が最小になるようにつなぎます。無向辺は `SpanningTreeEdge{from, to, weight}` で作ります。

```cpp
const std::vector<sirkit::SpanningTreeEdge> edges{{0, 1, 4}, {1, 2, 1}, {0, 2, 2}};
const auto ids = sirkit::minimum_spanning_forest(3, edges); // {1, 2}
```

## `minimum_spanning_forest`

```text
std::vector<std::size_t> minimum_spanning_forest(int n, std::span<const SpanningTreeEdge> edges);
```

選んだ辺の番号を返します。頂点と辺の番号は0始まりです。出力は重みの昇順で、同じ重みでは入力順です。負の重み・多重辺・自己ループを許します。孤立頂点からは辺を選びません。

頂点数を `N`、辺数を `M` として、最悪時間は `O(N+M log(M+1))`、領域は `O(N+M)` です。重み差と辺番号を64ビットにまとめられる大きな入力では、辺の整列に基数ソートを使います。 最大重みと最小重みの差に必要なビット数と、辺番号に必要なビット数の合計が64を超える場合は比較ソートを使います。

[使用例](../Examples/minimum_spanning_tree_example.cpp) ／ [Verify](../Verify/current/submissions/minimum_spanning_tree.cpp)
