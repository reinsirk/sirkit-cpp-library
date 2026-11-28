# 回文最短路

辺のラベルを順に読んで回文になる歩道のうち、辺数が最小のものを求めます。同じ辺や頂点を繰り返し通れます。

`PalindromicShortestPaths(n, edges, undirected=false)` に頂点数と辺の列を渡します。辺は `PalindromicPathEdge{from, to, label}` で、頂点は 0 始まり、ラベルは `uint8_t` です。既定は有向グラフです。

`distance(from, to)` は最短の辺数を返します。該当する歩道がなければ `-1`、同じ頂点なら `0` です。`path(from, to)` は通る頂点列 `vertices` と入力順の辺 ID 列 `edges` を返します。見つからないときは `nullopt` です。

```cpp
const std::vector<sirkit::PalindromicPathEdge> edges{{0, 1, 'a'}, {1, 2, 'a'}};
const sirkit::PalindromicShortestPaths paths(3, edges);
assert(paths.distance(0, 2) == 2);
```

ラベルの種類数を σ、ラベル c の辺数を m_c とすると、前計算は `O((σ + 1)n² + Σm_c²)` 時間と `O(n² + m)` 領域です。距離は `O(1)`、長さ ℓ の歩道の復元は `O(ℓ + 1)` です。

[使用例](../Examples/palindromic_shortest_paths_example.cpp) · [有向グラフの Verify](https://atcoder.jp/contests/abc394/tasks/abc394_e) · [無向グラフの Verify](https://atcoder.jp/contests/abc197/tasks/abc197_f)
