# ブロックカット森

無向グラフを、元の頂点と点二重連結ブロックからなる森に変換します。どの関節点を経由するか、どのブロック内で迂回できるかを木の経路で調べられます。[本体](../Graph/Block_Cut_Forest.cpp)

## `block_cut_forest`

```text
Graph block_cut_forest(int n, std::span<const std::pair<int, int>> edges);
```

頂点番号が `[0,n)` の無向グラフを渡します。元の頂点番号はそのままで、`n` 以上の番号はブロックです。ブロックの隣接リストに、そのブロックに属する元の頂点が入ります。番号や隣接順は規定しません。

孤立頂点は森でも孤立します。自己辺は1辺ごとに別のブロックになり、平行辺は同じブロックに入ります。元の辺数を $m$ として、構築は $O(n+m)$ 時間・領域です。

## 使用例

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 2},
                       std::pair{2, 0}, std::pair{2, 3}};
const auto forest = sirkit::block_cut_forest(4, edges);
// {0,1,2} と {2,3} のブロックが、元の頂点2でつながる。
```

同じ連結成分にある元の頂点 `a,b,c` について、`a` から `c` への単純パスが `b` を通れるかを判定できます。森で `b` から `a-c` の経路までの距離が1以下なら通れます。

[使用例](../Examples/block_cut_forest_example.cpp)・[AtCoder Verify](../Verify/current/submissions/block_cut_forest.cpp)・[Low Link](low_link.md)
