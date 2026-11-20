# 強連結成分分解

`scc_graph`は有向グラフを強連結成分に分けます。成分を1頂点にまとめると閉路のない有向グラフ（縮約DAG）になります。[本体](../Graph/SCC.cpp)・[使用例](../Examples/scc_example.cpp)を参照してください。

型名・コンストラクタ・`add_edge(int,int)`・`scc()`の呼び出しと戻り値は[ACL](https://atcoder.github.io/ac-library/production/document_ja/scc.html)に合わせています。型は`sirkit::scc_graph`です。

## API

### scc_graph

```text
scc_graph();
explicit scc_graph(int n);
explicit scc_graph(const Graph& g);
scc_graph(const scc_graph&);
scc_graph(scc_graph&& other) noexcept;
```

空、n頂点の未分解グラフ、またはGraphから分解済みグラフを作ります。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** 空・n頂点の準備はO(1)、Graphからの構築・分解はO(N+M)。コピーは保持量に比例。

### add_edge

```text
void add_edge(int u, int v);
```

有向辺u→vを追加し、既存の分解結果を無効にします。自己辺・多重辺を許します。

**制約:** u,vは0..N-1。

**計算量:** 償却O(1)。

### scc

```text
std::vector<std::vector<int>> scc();
```

分解し、トポロジカル順の`std::vector<std::vector<int>>`を返す。群内は頂点番号の昇順。

**制約:** 構築済みの入力が共通の前提を満たすこと。

**計算量:** O(N+M)時間・追加領域。

### build

```text
int build();
```

成分IDだけが必要な場合に、頂点群を作らず分解して成分数を返す。

**制約:** 入力グラフ・関数・比較器の前提は下記の共通条件に従います。

**計算量:** O(N+M)時間・追加領域。

### id

```text
int id(int v) const;
```

頂点が属する強連結成分の番号を返します。

**制約:** scc()またはbuild()後で、以後辺を追加していないこと。頂点vは有効な番号です。

**計算量:** O(1)。

### ids

```text
const std::vector<int>& ids() const;
```

強連結成分番号の配列へのconst参照を返します。

**制約:** scc()またはbuild()後で、以後辺を追加していないこと。頂点vは有効な番号です。

**計算量:** O(1)。

### component_count

```text
int component_count() const;
```

分解した連結成分の数を返します。

**制約:** scc()またはbuild()後で、以後辺を追加していないこと。頂点vは有効な番号です。

**計算量:** O(1)。

### condensation

```text
Graph condensation(bool unique_edges = true) const;
```

縮約Graphを返す。自己辺を除き、既定では多重辺も除く。

**制約:** scc()またはbuild()後で、以後辺を追加していないこと。頂点vは有効な番号です。

**計算量:** O(N+M)時間・追加領域。

### size

```text
int size() const noexcept;
```

頂点数を返します。

**制約:** なし。

**計算量:** O(1)。

### edge_count

```text
std::size_t edge_count() const noexcept;
```

登録した元辺の数を返します。

**制約:** なし。

**計算量:** O(1)。

### reserve_edges

```text
void reserve_edges(std::size_t count);
```

指定件数を保持できる辺の記録容量を予約します。

**制約:** 指定件数は内部の容量上限以内です。

**計算量:** 再確保する場合は現在の保持要素数に比例します。

### reset

```text
void reset(int n = 0);
```

旧状態を破棄し、指定頂点数の空グラフへ戻します。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** 旧領域の破棄費用。

### swap

```text
void swap(scc_graph& other) noexcept;
```

保持するグラフ・結果・状態を交換します。

**制約:** なし。

**計算量:** O(1)。


```cpp
scc_graph graph(3);
graph.add_edge(0,1);
graph.add_edge(1,0);
graph.add_edge(1,2);
auto groups = graph.scc(); // {{0,1},{2}}。
bool same = graph.id(0)==graph.id(1); // true。
Graph dag = graph.condensation();
```

頂点番号は0..N-1で、自己辺・多重辺を許します。`scc()`は呼ぶたびに分解し直します。成分IDや縮約グラフの照会は`scc()`または`build()`後に行い、辺を追加したら再分解します。成分番号は縮約DAGのトポロジカル順で、異なる成分間の辺u→vではid(u)<id(v)です。

N頂点・M辺として`scc()`・`build()`・`condensation()`はO(N+M)時間・追加領域、辺追加は償却O(1)です。

コピーは独立、move元は空の未分解グラフです。`build()`中の確保失敗は旧結果を保ちますが、追加済みの辺は取り消しません。`scc()`の頂点群作成に失敗した場合、分解結果は更新されていることがあります。`ids()`の参照は再分解・代入・破棄で無効になります。既定では端点と結果の有効性を検査し、`SKIP_RUNTIME_CHECKS`で除きます。内部容量・添字上限は維持します。

[独立参照](../Verify/current/tests/new_scc_two_sat_reference_test.cpp)・[Library Checker](https://judge.yosupo.jp/problem/scc)

## 構築・状態と補助型

### operator=

```text
scc_graph& operator=(scc_graph other) noexcept;
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用を加えます。
