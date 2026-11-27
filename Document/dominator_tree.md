# 支配木

有向グラフで `root` から `v` へ向かうすべての経路が `u` を通るとき、`u` は `v` を支配します。`dominator_tree` は、各頂点を支配する自身以外の頂点のうち、木で最も近い頂点を求めます。

```cpp
const std::vector<std::vector<int>> graph{{1, 2}, {3}, {3}, {4}, {}, {}};
const auto parent = sirkit::dominator_tree(graph, 0); // {0, 0, 0, 0, 3, -1}
```

## `dominator_tree`

```text
std::vector<int> dominator_tree(const std::vector<std::vector<int>> &graph, int root);
```

`graph[v]` に頂点 `v` の出辺の行き先を入れます。頂点番号は0始まりです。`root` 自身には `root`、到達不能な頂点には `-1` を返します。自己ループと多重辺も扱えます。

頂点数を `N`、辺数を `M` として、時間は `O((N+M) log(N+1))`、領域は `O(N+M)` です。

[使用例](../Examples/dominator_tree_example.cpp) ／ [Verify](../Verify/current/submissions/dominator_tree.cpp)

参考 [Luzhiled](https://ei1333.github.io/library/graph/others/dominator-tree.hpp)
