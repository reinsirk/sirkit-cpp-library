# 二部グラフの判定と2色塗り

無向グラフを2色に塗り分けます。隣り合う頂点をすべて異なる色にできない場合は、奇数長の閉路を返します。[本体](../Graph/Bipartite_Coloring.cpp)

`bipartite_coloring(n, edges)` に頂点数と `std::pair<int, int>` の辺列を渡します。頂点番号は $0$ 始まりです。非連結グラフ、自己辺、多重辺も扱えます。

- `is_bipartite()` が真なら `color[v]` に $0$ または $1$ が入ります。
- 偽なら `color` は空で、`odd_cycle` に巡回順の頂点列 `vertices` と辺番号列 `edge_ids` が入ります。辺番号は入力順です。

判定・復元に $O(n+m)$ 時間・領域を使い、`is_bipartite()` は $O(1)$ です。

## 使用例

```cpp
const std::array path{std::pair{0, 1}, std::pair{1, 2}};
const auto result = sirkit::bipartite_coloring(3, path);
// color == {0, 1, 0}。辺 {2, 0} を加えると長さ3の奇数閉路を返す。
```

[使用例](../Examples/bipartite_coloring_example.cpp)・[判定のVerify](../Verify/current/submissions/bipartite_coloring.cpp)・[最小頂点被覆のVerify](../Verify/current/submissions/bipartite_coloring_vertex_cover.cpp)
