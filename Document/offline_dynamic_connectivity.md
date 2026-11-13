# オフライン動的連結性

`OfflineDynamicConnectivity`は、無向グラフの辺追加・削除と照会を記録し、最後にsolveでまとめて解きます。連結判定・成分サイズ・成分数・成分の頂点重み和に対応します。[本体](../Graph/Offline_Dynamic_Connectivity.cpp)・[使用例](../Examples/offline_dynamic_connectivity_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `OfflineDynamicConnectivity(n)` / `(span<const int64_t> weights)` | n頂点・初期値0／指定した初期値で構築。 |
| `add_edge(u,v)` / `remove_edge(id)` | 辺を追加してsize_tのIDを返す／その辺を削除する。 |
| `add_vertex(v,delta)` | 以降の照会に対し頂点値へdeltaを加算する。 |
| `query_same(u,v)` / `query_size(v)` | その時点で連結か／成分サイズの照会を記録し、回答添字を返す。 |
| `query_component_count()` / `query_component_sum(v)` | 成分数／vの成分の頂点和の照会を記録し、回答添字を返す。 |
| `solve() const` | 照会順のvector<std::int64_t>を返す。連結判定は0/1。 |
| `size()` / `empty()` | 頂点数／0頂点か。 |
| `edge_count()` / `addition_count()` / `query_count()` | 削除済み込みの追加辺数／頂点加算数／照会数。 |
| `reserve_edges(n)` / `reserve_additions(n)` / `reserve_queries(n)` | 対応する記録容量を予約。 |
| `clear_history()` | 頂点数と構築時の初期値を残し、全ての辺・加算・照会を消す。 |
| `reset(n)` / `reset(weights)` / `clear()` | 再初期化／初期値を指定して再初期化／0頂点へ。 |

```cpp
OfflineDynamicConnectivity graph(4);
auto edge = graph.add_edge(0,1);
auto first = graph.query_same(0,1);
graph.remove_edge(edge);
auto second = graph.query_same(0,1);
auto answers = graph.solve(); // answers[first]==1、answers[second]==0。
```

## 履歴と計算量

solveは記録を消しません。何度でも実行でき、後から操作を追加して再solveできます。照会なしなら空vector、0頂点でも成分数の照会は可能で回答0です。clear_historyは加算後の値を新たな初期値にする操作ではありません。

辺が有効な期間を区間分割し、内部で巻き戻せるUnion-Findを使います。外部から任意時点へ戻すAPIはありません。頂点数N、追加辺数E、頂点加算数A、照会数Qとして、記録は償却O(1)、solveはO(N+(E+A+Q)log(Q+1)log(N+1))時間・O(N+(E+A)log(Q+1)+Q)作業領域です。構築はO(N)、件数の取得はO(1)です。

## 入力と所有

頂点は0..N-1で、自己辺・多重辺を許します。同じ両端の辺も別IDで、一方の削除は他方に影響しません。辺IDは有効な辺を削除するとき一度だけ使えます。無関係な別のオブジェクトのIDは渡さないでください。コピー時までのIDはコピー先でも同じ記録を指しますが、その後の操作は独立します。

頂点値は負でもよく、内部の加算順も含め全てstd::int64_tに収まることが前提です。初期化・履歴消去後の新しい記録には、古い辺ID・回答添字を流用しません。

コピーは全記録を独立所有し、コピーの費用は保持量に比例します。move元は0頂点・記録なし、move・swapはO(1)。確保失敗時は元の内容を保ち、constなsolveも記録を変更しません。既定では頂点番号と有効な辺IDを検査し、`SKIP_RUNTIME_CHECKS`で除きます。内部容量・添字保護は維持します。

[Library Checker Dynamic Graph Vertex Add Component Sum](https://judge.yosupo.jp/problem/dynamic_graph_vertex_add_component_sum)と、連結判定・サイズ・再solve等の独立参照で検証します。
