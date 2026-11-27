# ST 番号付け

無向グラフの辺を順位が小さい頂点から大きい頂点へ向けます。すべての頂点が始点から終点へ向かうパスのいずれかに載る順位を求めます。

```cpp
const std::array edges{std::pair{1, 2}, std::pair{2, 0}};
const auto ranks = sirkit::st_numbering(3, 1, 0, edges);  // {2, 0, 1}
```

## `st_numbering`

```text
std::optional<std::vector<int>> st_numbering(
    int n, int source, int sink, std::span<const std::pair<int, int>> edges);
```

無向辺を `{u, v}` の列で渡します。`(*ranks)[v]` は頂点 `v` の順位で、始点が0、終点が `n-1` です。条件を満たす順位がなければ `std::nullopt` になります。

頂点番号は `[0,n)` です。始点と終点の間に辺がない場合も使えます。多重辺を含められ、自己辺は無視します。空グラフと同一端点の結果は `std::nullopt` です。

反復DFSで、始点と終点を結ぶ辺を足したグラフの二重連結性を調べます。2頂点の場合は元のグラフに2頂点を結ぶ辺が必要です。頂点数を $n$、辺数を $m$ とすると $O(n+m)$ 時間・領域です。

[使用例](../Examples/st_numbering_example.cpp) ／ [Verify](../Verify/current/submissions/st_numbering.cpp) ／ [Tarjan (1986) の原文 PDF](https://www.cs.princeton.edu/courses/archive/fall03/cs528/handouts/two_stremlined.pdf)
