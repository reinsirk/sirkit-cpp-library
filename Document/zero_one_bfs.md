# 0-1 BFS

辺の重みが0か1のとき、始点からの最短距離と経路を求めます。グリッドのように隣接頂点がその場で分かる場合は辺列を保存せずに使えます。

```cpp
auto result = sirkit::zero_one_bfs(2, 0, [](int v, auto visit)
                                  { if (v == 0) visit(1, 1); });
```

## `zero_one_bfs`

```text
template <class Neighbors>
ZeroOnePaths zero_one_bfs(int n, int source, Neighbors &&neighbors);
```

`neighbors(v, visit)` から頂点 `v` の各有向辺について `visit(行先, 重み)` を呼びます。`visit` は `neighbors` の呼び出し中だけ使えます。重みは0または1、頂点番号は `[0,n)` です。`distance` に距離、`parent` に経路上の直前の頂点を返します。到達不能なら両方とも `-1` で、始点の親は始点自身です。辺列を次数に比例する時間で列挙できれば時間・領域ともに $O(n+m)$ です。

[使用例](../Examples/zero_one_bfs_example.cpp) ／ [Verify](../Verify/current/submissions/zero_one_bfs_wizard.cpp)
