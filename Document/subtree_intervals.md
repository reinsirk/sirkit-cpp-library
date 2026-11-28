# 部分木の区間

親配列で表した森をDFSの行きがけ順に並べます。各部分木は連続区間になるため、Fenwick木やセグメント木で部分木の集約を扱えます。[本体](../Graph/Subtree_Intervals.cpp)

| 操作 | 内容 |
| --- | --- |
| `SubtreeIntervals(parents)` | 親配列から構築。根の親は `-1` |
| `range(v)` | 頂点 `v` を根とする部分木の半開区間 |
| `index(v)` / `vertex_at(i)` | 頂点の行きがけ順の位置、または位置にある頂点 |
| `contains(u, v)` | `v` が `u` の部分木に属するか判定 |
| `order()` | 行きがけ順の頂点を `std::span<const int>` で参照 |

```cpp
const std::array parents{-1, 0, 0, 1, 1};
const sirkit::SubtreeIntervals index(parents);
assert(index.range(1) == (std::pair{1, 4}));
assert(index.vertex_at(2) == 3);
```

頂点と位置は0始まりです。根と子はそれぞれ頂点番号の昇順に訪問します。構築は時間・領域とも $O(n)$、各操作は $O(1)$ です。

[実行できる使用例](../Examples/subtree_intervals_example.cpp)・[Library Checker Verify](../Verify/current/submissions/vertex_add_subtree_sum.cpp)
