# 重心分解

`CentroidDecomposition`は、無向木を「取り除くと各成分が半分以下になる頂点＝重心」で分割し、O(log(N+1))段の重心木を作ります。頂点への印付けと最寄りの印の照会などに使えます。[本体](../Graph/Centroid_Decomposition.cpp)・[使用例](../Examples/centroid_decomposition_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `CentroidDecomposition(graph,store_distances=true)` / `build(...)` | 分解を構築／再構築する。入力を変更・保持しない。 |
| `root()` / `parent(v)` / `depth(v)` | 重心木の根／親／深さ。根の親、空の根は-1。 |
| `component_size(v)` | vを重心として除去した時点の成分サイズ。 |
| `ancestors(v)` | 重心木の根からvまでのAncestor列へのconst参照。最後は{v,0}。 |
| `distance(u,v)` | 元の木の辺数距離をstd::int64_tで返す。 |
| `parents()` / `depths()` / `order()` | 結果配列へのconst参照。orderでは親が子より先。 |
| `centroid_tree()` | 重心木の無向隣接リストを所有値で返す。 |
| `size()` / `empty()` / `stores_distances()` | 頂点数／空か／距離を保存しているか。 |
| `clear()` / `swap(other)` | 空にする／結果を交換する。 |

```cpp
Graph graph{{1}, {0,2}, {1,3}, {2}};
CentroidDecomposition decomposition(graph);
auto distance = decomposition.distance(0,3); // 3。
for (auto a : decomposition.ancestors(3)) {
    // a.vertexは祖先の重心、a.distanceは元の木での3からの距離。
}
```

## 距離情報の使い方

Ancestorは{vertex,distance}です。距離は重心木ではなく**元の木での辺数**で、各祖先列の長さはO(log(N+1))です。最寄りの印を管理する場合、各重心について「印からの最小距離」を持ち、印付け・照会で祖先列だけをたどります。削除を行わない場合、各操作はO(log(N+1))です。具体的なコードは使用例にあります。

既定では距離情報も構築し、O(N log(N+1))時間・領域です。`CentroidDecomposition(graph,false)`または`build(graph,false)`なら距離を保存せず、領域O(N)です。この場合ancestors・distanceは使えません。distanceはO(log(N+1))、centroid_treeはO(N)、結果配列の参照・単点取得はO(1)です。

## 入力・所有

頂点0..N-1の単純無向木が前提です。空の木も許します。全探索は反復で、長い鎖でもNに比例する再帰スタックを使いません。入力は構築後に破棄・変更できます。

コピーは独立、move元は空、swapはO(1)です。コピー代入・buildに失敗した場合は旧結果を維持します。結果への参照は再構築・代入・clear・破棄まで有効です。

既定では初回の成分探索に、頂点番号・循環・到達頂点数・逆向きの辺の検査を組み込みます。検査専用の全辺走査や、後の成分での繰り返し検査はしません。`SKIP_RUNTIME_CHECKS`でこれらと頂点・距離保存の検査を除き、頂点数の表現上限は維持します。

数学的参考は[Nyaan](https://nyaannyaan.github.io/library/tree/centroid-decomposition.hpp.html)、機能上の参考は[Luzhiled](https://ei1333.github.io/library/graph/tree/centroid-decomposition.hpp.html)です。入力を保持しない反復構築として独立に記述しています。
