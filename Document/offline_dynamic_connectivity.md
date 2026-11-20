# オフライン動的連結性

`OfflineDynamicConnectivity`は、無向グラフの辺追加・削除と照会を記録し、最後にsolveでまとめて解きます。連結判定・成分サイズ・成分数・成分の頂点重み和に対応します。[本体](../Graph/Offline_Dynamic_Connectivity.cpp)・[使用例](../Examples/offline_dynamic_connectivity_example.cpp)を参照してください。

## API

### OfflineDynamicConnectivity

```text
explicit OfflineDynamicConnectivity(int n = 0);
explicit OfflineDynamicConnectivity(std::span<const std::int64_t> weights);
OfflineDynamicConnectivity(const OfflineDynamicConnectivity&);
OfflineDynamicConnectivity(OfflineDynamicConnectivity&& other) noexcept;
```

n頂点のグラフを作ります。初期重みは0か、指定した値になります。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** O(N)時間・領域。再構築は旧記録の破棄費用も加えます。

### add_edge

```text
std::size_t add_edge(int u, int v);
```

その時点で辺を追加し、削除指定に使うstd::size_tの辺IDを返します。

**制約:** u,vは0..N-1。自己辺・多重辺も可。

**計算量:** 償却O(1)。

### remove_edge

```text
void remove_edge(std::size_t id);
```

辺を追加してsize_tのIDを返す／その辺を削除する。

**制約:** このオブジェクトで追加され、まだ削除していない辺IDであること。

**計算量:** 償却O(1)。

### add_vertex

```text
void add_vertex(int v, std::int64_t delta);
```

以降の照会に対し頂点値へdeltaを加算する。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** 償却O(1)。

### query_same

```text
std::size_t query_same(int u, int v);
```

その時点で2頂点が連結かの照会を記録し、回答添字を返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** 償却O(1)。

### query_size

```text
std::size_t query_size(int v);
```

その時点の頂点の成分サイズの照会を記録し、回答添字を返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** 償却O(1)。

### query_component_count

```text
std::size_t query_component_count();
```

その時点の成分数の照会を記録し、回答添字を返します。

**制約:** なし。0頂点でも照会できます。

**計算量:** 償却O(1)。

### query_component_sum

```text
std::size_t query_component_sum(int v);
```

その時点の頂点の成分重み和の照会を記録し、回答添字を返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** 償却O(1)。

### solve

```text
std::vector<std::int64_t> solve() const;
```

照会を記録した順に回答を並べ、vector<std::int64_t>で返します。連結判定の結果は0か1です。

**制約:** 構築済みの入力が共通の前提を満たすこと。

**計算量:** O(N+(E+A+Q)log(Q+1)log(N+1))時間・O(N+(E+A)log(Q+1)+Q)追加領域。

### size

```text
int size() const noexcept;
```

頂点数を返します。

**制約:** なし。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

頂点数が0ならtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### edge_count

```text
std::size_t edge_count() const noexcept;
```

登録した元辺の数を返します。

**制約:** なし。

**計算量:** O(1)。

### addition_count

```text
std::size_t addition_count() const noexcept;
```

記録した頂点加算の個数を返します。

**制約:** なし。

**計算量:** O(1)。

### query_count

```text
std::size_t query_count() const noexcept;
```

記録した照会の個数を返します。

**制約:** なし。

**計算量:** 償却O(1)。

### reserve_edges

```text
void reserve_edges(std::size_t cap);
```

指定件数を保持できる辺の記録容量を予約します。

**制約:** 指定件数は内部の容量上限以内です。

**計算量:** 再確保する場合は現在の保持要素数に比例します。

### reserve_additions

```text
void reserve_additions(std::size_t cap);
```

頂点加算の記録容量を予約します。

**制約:** 指定件数は内部の容量上限以内です。

**計算量:** 再確保する場合は現在の保持要素数に比例します。

### reserve_queries

```text
void reserve_queries(std::size_t cap);
```

照会の記録容量を予約します。

**制約:** 指定件数は内部の容量上限以内です。

**計算量:** 再確保する場合は現在の保持要素数に比例します。

### clear_history

```text
void clear_history() noexcept;
```

頂点数と構築時の初期値を残し、全ての辺・加算・照会を消す。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** 保持記録の消去・破棄に比例します。

### reset

```text
void reset(int n = 0);
void reset(std::span<const std::int64_t> weights);
```

旧状態を破棄し、指定頂点数の空グラフへ戻します。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** O(N)時間・領域。再構築は旧記録の破棄費用も加えます。

### clear

```text
void clear() noexcept;
```

領域を解放して空の状態へ戻します。

**制約:** なし。

**計算量:** 保持記録の消去・破棄に比例します。


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

頂点数N、追加辺数E、頂点加算数A、照会数Qとして、記録は償却O(1)、solveはO(N+(E+A+Q)log(Q+1)log(N+1))時間・O(N+(E+A)log(Q+1)+Q)作業領域です。構築はO(N)、件数の取得はO(1)です。

## 入力と所有

頂点は0..N-1で、自己辺・多重辺を許します。同じ両端の辺も別IDで、一方の削除は他方に影響しません。辺IDは有効な辺を削除するとき一度だけ使えます。無関係な別のオブジェクトのIDは渡さないでください。コピー時までのIDはコピー先でも同じ記録を指しますが、その後の操作は独立します。

頂点値は負でもよく、内部の加算順も含め全てstd::int64_tに収まることが前提です。初期化・履歴消去後の新しい記録には古い辺ID・回答添字を流用しません。

コピーは全記録を独立所有し、コピーの費用は保持量に比例します。move元は0頂点・記録なし、move・swapはO(1)。確保失敗時は元の内容を保ち、constなsolveも記録を変更しません。既定では頂点番号と有効な辺IDを検査し、`SKIP_RUNTIME_CHECKS`で除きます。内部容量・添字保護は維持します。

[Library Checker Dynamic Graph Vertex Add Component Sum](https://judge.yosupo.jp/problem/dynamic_graph_vertex_add_component_sum)を含む検証記録は[Audit](../Audit/)に置きます。

## 構築・状態と補助型

### operator=

```text
OfflineDynamicConnectivity& operator=(const OfflineDynamicConnectivity& other);
OfflineDynamicConnectivity& operator=(OfflineDynamicConnectivity&& other) noexcept;
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用を加えます。

### swap

```text
void swap(OfflineDynamicConnectivity& other) noexcept;
void swap(OfflineDynamicConnectivity& a, OfflineDynamicConnectivity& b) noexcept;
```

保持する状態を交換します。

**制約:** なし。

**計算量:** O(1)。
