# Kruskal 再構築木

辺を重み順に追加し、連結成分が併合されるたびに内部ノードを作ります。2頂点が初めて連結する重みや、重みのしきい値を指定したときの成分が分かります。[本体](../Graph/Kruskal_Reconstruction_Tree.cpp)

`KruskalReconstructionTree(n, edges, order)` に、$0$ 始まりの頂点数と `SpanningTreeEdge{from, to, weight}` の辺列を渡します。無向グラフで、重みは符号付き64ビットです。`KruskalOrder::increasing` が既定の昇順で、`decreasing` は降順。同じ重みの辺は入力順に処理します。

| 操作 | 意味 | 時間 |
| --- | --- | --- |
| `size()`、`node_count()` | 元の頂点数、併合ノードを含む数 | $O(1)$ |
| `node(i)` | 親・左右の子、含まれる元の頂点数、併合重み | $O(1)$ |
| `merge_node(u, v)` | 初めて同じ成分になるノード。非連結なら `-1` | $O(1)$ |
| `merge_weight(u, v)` | 併合重み。同じ頂点か非連結なら `std::nullopt` | $O(1)$ |
| `component_at(v, w)` | 昇順なら重みが `w` 以下、降順なら `w` 以上の辺で作る連結成分 | $O(\log^2(n+1))$ |

葉の番号は元の頂点番号です。親や子がなければ `-1`、葉の重みは `std::nullopt`。前処理は $O(n\log(n+1)+m\log(m+1))$ 時間、$O(n\log(n+1)+m)$ 領域です。

## 使用例

```cpp
const std::array edges{sirkit::SpanningTreeEdge{0, 1, 3},
                       sirkit::SpanningTreeEdge{1, 2, 4}};
const sirkit::KruskalReconstructionTree tree(3, edges);
// merge_weight(0, 2) == 4。重み3以下で頂点0の成分は {0, 1}。
```

[使用例](../Examples/kruskal_reconstruction_tree_example.cpp)・[Verify](../Verify/current/submissions/kruskal_reconstruction_tree.cpp)
