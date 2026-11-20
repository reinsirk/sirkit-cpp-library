# Low-linkと二重連結成分

`UndirectedLowLink`は、無向多重グラフの橋・関節点・二重連結成分を求めます。頂点番号は0始まりの`[0,n)`、辺は追加時のIDで区別します。

## API

### UndirectedLowLink

```text
explicit UndirectedLowLink(int n = 0);
UndirectedLowLink(const UndirectedLowLink&);
UndirectedLowLink(UndirectedLowLink&& other) noexcept;
```

空のn頂点グラフを作ります。コピーは独立、ムーブ元は空です。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** 構築はO(1)。コピーは保持領域に比例、ムーブはO(1)。

### reset

```text
void reset(int n = 0);
```

旧状態を破棄し、指定頂点数の空グラフへ戻します。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** 旧データの破棄費用。

### reserve_edges

```text
void reserve_edges(std::size_t m);
```

指定件数を保持できる辺の記録容量を予約します。

**制約:** 指定件数は内部の容量上限以内です。

**計算量:** 再確保する場合は現在の保持要素数に比例します。

### add_edge

```text
std::size_t add_edge(int u, int v);
```

無向辺を追加し、std::size_tの辺IDを返します。

**制約:** u,vは0..n-1。自己辺・平行辺も可。

**計算量:** 償却O(1)。

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

### edge

```text
std::pair<int, int> edge(std::size_t id) const;
```

指定した辺の追加時の端点を返します。

**制約:** idは登録済みの有効な辺番号です。分解結果の取得はbuild後に行います。

**計算量:** O(1)。

### build

```text
int build(bool collect_blocks = false);
```

グラフを分解し、元のグラフの連結成分数を返します。O(n+m)です。trueを指定すると点二重連結ブロックも求めます。

**制約:** build後で、以後辺を追加していないこと。頂点・辺番号は有効範囲です。

**計算量:** O(n+m)時間・領域。

### component_count

```text
int component_count() const;
```

分解した連結成分の数を返します。

**制約:** build後で、以後辺を追加していないこと。頂点・辺番号は有効範囲です。

**計算量:** O(1)。

### bridges

```text
const std::vector<std::size_t>& bridges() const;
```

橋の辺IDを昇順に並べた配列をconst参照で返します。

**制約:** build後で、以後辺を追加していないこと。頂点・辺番号は有効範囲です。

**計算量:** O(1)。

### is_bridge

```text
bool is_bridge(std::size_t e) const;
```

指定した辺が橋ならtrueを返します。

**制約:** idは登録済みの有効な辺番号です。分解結果の取得はbuild後に行います。

**計算量:** O(1)。

### articulation_points

```text
const std::vector<int>& articulation_points() const;
```

関節点番号を昇順に並べた配列をconst参照で返します。

**制約:** build後で、以後辺を追加していないこと。頂点・辺番号は有効範囲です。

**計算量:** O(1)。

### is_articulation

```text
bool is_articulation(int v) const;
```

指定した頂点が関節点ならtrueを返します。

**制約:** build後で、以後辺を追加していないこと。頂点・辺番号は有効範囲です。

**計算量:** O(1)。

### two_edge_component_count

```text
int two_edge_component_count() const;
```

2辺連結成分数を返す。O(1)。

**制約:** build後で、以後辺を追加していないこと。頂点・辺番号は有効範囲です。

**計算量:** O(1)。

### two_edge_component

```text
int two_edge_component(int v) const;
```

橋を除いた2辺連結成分の番号を返します。

**制約:** build後で、以後辺を追加していないこと。頂点・辺番号は有効範囲です。

**計算量:** O(1)。

### two_edge_components

```text
std::vector<std::vector<int>> two_edge_components() const;
```

2辺連結成分ごとの頂点群を独立した値として返します。

**制約:** build後で、以後辺を追加していないこと。頂点・辺番号は有効範囲です。

**計算量:** O(n+m)時間・領域。

### bridge_forest

```text
Graph bridge_forest() const;
```

2辺連結成分を頂点、橋を辺とした`Graph`を返す。O(n+m)。頂点番号は上記の成分番号。

**制約:** build後で、以後辺を追加していないこと。頂点・辺番号は有効範囲です。

**計算量:** O(n+m)時間・領域。

### biconnected_edge_components

```text
const std::vector<std::vector<std::size_t>>& biconnected_edge_components() const;
```

点二重連結ブロックごとの辺ID群へのconst参照。build(true)後、O(1)。

**制約:** build(true)後で、以後辺を追加していないこと。

**計算量:** O(1)。

### biconnected_vertex_components

```text
std::vector<std::vector<int>> biconnected_vertex_components(bool include_isolated = true) const;
```

各ブロックの頂点群を返す。trueなら孤立頂点も単独群で加える。build(true)後、O(n+m)。

**制約:** build(true)後で、以後辺を追加していないこと。

**計算量:** O(n+m)時間・領域。

### swap

```text
void swap(UndirectedLowLink& other) noexcept;
```

保持するグラフ・結果・状態を交換します。

**制約:** なし。

**計算量:** O(1)。


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

結果を照会するにはbuildが必要です。辺を追加した後は再度buildしてください。橋のIDと関節点番号は昇順ですが、成分番号・成分群やブロックの順序は規定しません。値として返す群は独立したコピーです。const参照は再度のbuild、reset、代入で無効になります。

平行辺は別IDとして処理します。自己辺はそれぞれ独立した1辺ブロックとし、同じ頂点だけの頂点ブロックが複数できる場合があります。孤立頂点は辺ブロックには含めません。この自己辺の扱いは、自己辺を許さないLibrary Checkerの点二重連結成分問題と区別してください。

構築・分解はO(n+m)時間・領域です。

コピーは独立所有、ムーブ元は空の未分解グラフです。build中に例外が出た場合は呼び出し前の状態を保ちます。`SKIP_RUNTIME_CHECKS`で頂点・辺番号・結果の有効性の検査を除きます。内部の容量保護や、橋・関節点を決める数学的な比較は維持します。

[独立参照](../Verify/current/tests/new_low_link_reference_test.cpp) · [確保失敗の検証](../Verify/current/tests/low_link_components_allocation_test.cpp) · [2辺連結成分](https://judge.yosupo.jp/problem/two_edge_connected_components) · [点二重連結成分](https://judge.yosupo.jp/problem/biconnected_components)

## 構築・状態と補助型

### operator=

```text
UndirectedLowLink& operator=(UndirectedLowLink other) noexcept;
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用を加えます。
