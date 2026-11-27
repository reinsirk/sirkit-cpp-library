# 辺追加と強連結成分

空の有向グラフに辺を順に加え、強連結成分が併合する時刻をまとめて求めます。追加順を先にすべて渡します。

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 0}};
const auto events = sirkit::incremental_scc_merges(3, edges);
// 3本目の追加後に3頂点が同じ成分になり、2件の併合が発生する。
```

## `incremental_scc_merges`

```text
std::vector<IncrementalSccMerge> incremental_scc_merges(
    int n, std::span<const std::pair<int, int>> edges);
```

時刻 `time` は追加した辺の本数を表します。頂点番号は0始まりで、時刻0では各頂点が自分の成分の代表です。`{time, leader, absorbed}` は、代表が `absorbed` の成分を `leader` の成分に併合する操作です。同じ時刻の操作も返却順に適用します。自己辺と重複辺を渡せます。

頂点数を $n$、追加する辺数を $m$ とすると $O(n+m\log(m+1))$ 時間・$O(n+m)$ 領域です。このファイルは `scc_graph` と `UnionFind` も同梱しています。 [SCC](scc.md) ／ [UnionFind](union_find.md)

[使用例](../Examples/incremental_scc_example.cpp) ／ [Verify](../Verify/current/submissions/incremental_scc.cpp) ／ [時刻分割・縮約の参考文献](https://doi.org/10.4230/LIPIcs.SWAT.2026.17)
