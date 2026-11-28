# 補グラフの最短経路

元の辺が存在しない頂点どうしを結ぶグラフを補グラフと呼びます。無向補グラフ上の距離、最短経路の復元に使う直前の頂点、最短経路数を求めます。[本体](../Graph/Complement_Shortest_Paths.cpp)

## `complement_bfs`

```text
ComplementBfsResult complement_bfs(int n, std::span<const std::pair<int, int>> edges,
                                  int source = 0, std::uint64_t modulus = 0);
```

頂点数 `n` と元の無向辺を渡します。頂点番号は `[0,n)`。自己辺と多重辺は結果に影響しません。空のグラフでは `source=0` を指定します。

戻り値には頂点番号順に以下の配列が入ります。

- `distance[v]` は始点からの辺数距離。到達できなければ `-1`。
- `parent[v]` は最短経路で `v` の直前に訪れる頂点。始点と到達不能頂点では `-1`。
- `paths[v]` は最短経路数を `modulus` で割った余り。到達できなければ `0`。`modulus=0` なら配列は空。

補グラフの辺を列挙せず、頂点数 $n$、元の辺数 $m$ に対し $O(n+m)$ 時間・領域です。

## 使用例

```cpp
const std::array removed{std::pair{0, 3}};
const auto result = sirkit::complement_bfs(4, removed, 0, 998244353);
// 頂点3への最短経路は 0→1→3 と 0→2→3。
// result.distance[3] == 2、result.paths[3] == 2。
```

[使用例](../Examples/complement_shortest_paths_example.cpp)・[AtCoder Verify](../Verify/current/submissions/complement_shortest_paths.cpp)
