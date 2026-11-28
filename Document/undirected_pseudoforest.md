# なもりグラフの分解

連結成分ごとに閉路が高々1個の無向グラフを、閉路とそこにつながる木に分けます。2頂点間の単純パスが一意かどうかや、最短距離も求められます。[本体](../Graph/Undirected_Pseudoforest.cpp)

`UndirectedPseudoforest::build(n, edges)` に頂点数と `std::pair<int, int>` の辺列を渡します。頂点番号は $0$ 始まりです。自己辺と多重辺を扱えます。閉路が2個以上ある連結成分を含む場合は `std::nullopt` を返します。構築は $O(n+m)$ 時間・領域です。

| 操作 | 戻り値 | 時間 |
| --- | --- | --- |
| `size()`、`component_count()`、`cycle_count()` | 頂点数、連結成分数、閉路数 | $O(1)$ |
| `component(v)` | 連結成分を代表する頂点 | $O(1)$ |
| `root(v)`、`depth(v)` | 所属する木の根、根までの辺数。閉路がある成分では根は閉路上の頂点 | $O(1)$ |
| `cycle_id(v)` | 連結成分の閉路番号。閉路がなければ `-1` | $O(1)$ |
| `cycle(id)` | 巡回順の `vertices` と `edge_ids`。辺番号は入力順 | $O(1)$ |
| `unique_path(u, v)` | 辺番号で区別して単純パスがちょうど1本なら `true` | $O(1)$ |
| `distance(u, v)` | 最短パスの辺数。非連結なら `-1` | $O(\log(n+1))$ |

`cycle(id)` が返す各列は本体を参照する `std::span` です。辺 `edge_ids[i]` は `vertices[i]` と次の頂点を結びます。

## 使用例

```cpp
using Edge = std::pair<int, int>;
const std::array edges{Edge{0, 1}, Edge{1, 2}, Edge{2, 0}, Edge{1, 3}};
const auto graph = sirkit::UndirectedPseudoforest::build(4, edges).value();
// 1から3へのパスは一意。0から3へは閉路のどちら側からも行ける。
```

[使用例](../Examples/undirected_pseudoforest_example.cpp)・[単純パスのVerify](../Verify/current/submissions/undirected_pseudoforest.cpp)・[連結成分のVerify](../Verify/current/submissions/undirected_pseudoforest_components.cpp)
