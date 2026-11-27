# 二部グラフの辺彩色

同じ頂点に接する辺が異なる色になるように彩色します。二部グラフでは最大次数が必要な最小色数です。

```cpp
const std::array edges{std::pair{0, 0}, std::pair{0, 1}, std::pair{1, 1}};
const auto result = sirkit::bipartite_edge_coloring(2, 2, edges);
// result.color_count == 2、result.colors は {0, 1, 0} など
```

## `bipartite_edge_coloring`

```text
BipartiteEdgeColoringResult bipartite_edge_coloring(
    int left, int right, std::span<const std::pair<int, int>> edges);
```

各辺 `{u, v}` の左端は `[0,left)`、右端は `[0,right)` です。多重辺も1本ずつ区別して彩色します。`colors[i]` は入力の `i` 番目の辺の色で、`[0,color_count)` の整数です。辺がなければ色数は0です。

左の頂点数を $L$、右の頂点数を $R$、辺数を $m$ とすると、$O(L+R+m\log(m+1))$ 時間・$O(L+R+m)$ 領域です。同じ側の頂点を先にまとめて、正則化に必要な補助辺を $O(m)$ 本に抑えます。

[使用例](../Examples/bipartite_edge_coloring_example.cpp) ／ [Verify](../Verify/current/submissions/bipartite_edge_coloring.cpp) ／ [Alonの原論文](https://web.math.princeton.edu/~nalon/PDFS/lex2.pdf)
