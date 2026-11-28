# マーク付き木の動的な直径

頂点をマーク・解除しながら、マークされた頂点間の最長距離とその端点を求めます。各辺の長さは $1$ です。[本体](../Graph/Tree_Marked_Diameter.cpp)

## `TreeMarkedDiameter`

`TreeMarkedDiameter(n, edges, initially_marked=false)` に、頂点番号が $0$ 始まりの無向木の辺を1回ずつ渡します。空の木は `n=0` とし、全頂点をマーク済みにして開始する場合は第3引数を `true` にします。$B=\lceil n/32\rceil$ とすると、構築の時間・領域は $O(n+B\log(B+1))$ です。

| 操作 | 内容 | 時間 |
| --- | --- | --- |
| `set_marked(v, marked=true)` | 頂点 `v` のマークを設定または解除する | $O(\log(n+1))$ |
| `toggle(v)` | マークを反転し、更新後の状態を返す | $O(\log(n+1))$ |
| `marked(v)`, `size()` | マークの有無、頂点数を返す | $O(1)$ |
| `diameter()` | 全マークの直径を返す | $O(1)$ |
| `subtree_diameter(v)` | 頂点 $0$ を根とし、`v` の部分木内のマークに限った直径を返す | $O(\log(n+1))$ |
| `farthest(v)` | `v` から最も遠いマーク済み頂点と距離を返す | $O(1)$ |

どの照会も、対象のマークが空なら `std::nullopt` を返します。直径の結果には `first`、`second`、`distance` があり、対象が1頂点なら両端点は同じです。最遠頂点の結果には `vertex` と `distance` があります。

```cpp
const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{1, 3}};
sirkit::TreeMarkedDiameter tree(4, edges);
tree.set_marked(0);
tree.set_marked(2);
assert(tree.diameter()->distance == 2);
assert(tree.subtree_diameter(1)->distance == 0);
```

[実行できる使用例](../Examples/tree_marked_diameter_example.cpp)・[AtCoder Verify](../Verify/current/submissions/tree_marked_diameter.cpp)・[直径をまとめる考え方](https://atcoder.jp/contests/abc460/editorial/21029)
