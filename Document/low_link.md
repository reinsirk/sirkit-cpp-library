# Low-linkと二重連結成分

`UndirectedLowLink`は、無向多重グラフの橋・関節点・二重連結成分を求めます。橋は取り除くと連結成分が増える辺、関節点は取り除くと連結成分が増える頂点です。頂点番号は0始まりの`[0,n)`、辺は追加時のIDで区別します。

## API

| API | 動作・計算量 |
| --- | --- |
| `UndirectedLowLink(n=0)` / `reset(n=0)` | 空のn頂点グラフを作る／戻す。構築O(1)、resetは旧データの解放を含む。 |
| `reserve_edges(m)` / `add_edge(u,v)` | m辺分を予約／無向辺を追加し`size_t`の辺IDを返す。追加は償却O(1)。 |
| `size()` / `edge_count()` / `edge(id)` | 頂点数／辺数／端点の`pair<int,int>`を返す。O(1)。 |
| `build(collect_blocks=false)` | 分解して元のグラフの連結成分数を返す。O(n+m)。trueなら点二重連結ブロックも求める。 |
| `component_count()` | 最後のbuildの連結成分数。O(1)。 |
| `bridges()` / `is_bridge(id)` | 橋の辺ID群へのconst参照／橋かを返す。O(1)。 |
| `articulation_points()` / `is_articulation(v)` | 関節点番号群へのconst参照／関節点かを返す。O(1)。 |
| `two_edge_component_count()` | 2辺連結成分数を返す。O(1)。 |
| `two_edge_component(v)` / `two_edge_components()` | 橋を除いた成分番号／成分ごとの頂点群を返す。O(1)／O(n+m)。 |
| `bridge_forest()` | 2辺連結成分を頂点、橋を辺とした`Graph`を返す。O(n+m)。頂点番号は上記の成分番号。 |
| `biconnected_edge_components()` | 点二重連結ブロックごとの辺ID群へのconst参照。build(true)後、O(1)。 |
| `biconnected_vertex_components(include_isolated=true)` | 各ブロックの頂点群を返す。trueなら孤立頂点も単独群で加える。build(true)後、O(n+m)。 |
| `swap(other)` | グラフと分解結果をO(1)で交換する。 |

```cpp
UndirectedLowLink graph(4);
graph.add_edge(0,1); graph.add_edge(1,2); graph.add_edge(2,0);
auto edge = graph.add_edge(2,3);
graph.build(true);
bool bridge = graph.is_bridge(edge);              // true
int count = graph.two_edge_component_count();      // 2
auto groups = graph.two_edge_components();        // {0,1,2}と{3}
auto blocks = graph.biconnected_vertex_components();
auto forest = graph.bridge_forest();
```

[完全な使用例](../Examples/low_link_example.cpp)を参照してください。

## 前提と性質

結果の照会にはbuildが必要です。辺の追加後は再buildしてください。橋のIDと関節点番号は昇順ですが、成分番号・成分群やブロックの順序は規定しません。群を返す値は独立したコピーで、const参照は再build・reset・代入で無効になります。

平行辺は別IDとして処理します。自己辺はそれぞれ独立した1辺ブロックとし、同じ頂点だけの頂点ブロックが複数できる場合があります。孤立頂点は辺ブロックには含めません。この自己辺の扱いは、自己辺を許さないLibrary Checkerの点二重連結成分問題と区別してください。

DFSは明示スタックを使うため、深い鎖でも再帰しません。発見順と祖先へ戻れる最小の発見順から橋・関節点を求め、橋に当たらない木辺の親参照を圧縮して2辺連結成分をまとめます。build(true)では辺スタックで点二重連結ブロックも確定します。構築はO(n+m)時間・領域です。

コピーは独立所有、ムーブ元は空の未分解グラフです。build中に例外が出た場合は呼び出し前の状態を保ちます。`SKIP_RUNTIME_CHECKS`で頂点・辺番号・結果の有効性の検査を除きます。内部の容量保護や、橋・関節点を決める数学的な比較は維持します。

[独立参照](../Verify/current/tests/new_low_link_reference_test.cpp) · [確保失敗の検証](../Verify/current/tests/low_link_components_allocation_test.cpp) · [2辺連結成分](https://judge.yosupo.jp/problem/two_edge_connected_components) · [点二重連結成分](https://judge.yosupo.jp/problem/biconnected_components)
