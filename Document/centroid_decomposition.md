# 重心分解

`CentroidDecomposition`は、無向木を重心で分割し、O(log(N+1))段の重心木を作ります。頂点への印付けと最寄りの印の照会などに使えます。[本体](../Graph/Centroid_Decomposition.cpp)・[使用例](../Examples/centroid_decomposition_example.cpp)を参照してください。

## API

### CentroidDecomposition

```text
CentroidDecomposition();
explicit CentroidDecomposition(const Graph& graph, bool store_distances = true);
CentroidDecomposition(const CentroidDecomposition&);
CentroidDecomposition(CentroidDecomposition&& other) noexcept;
```

無向木の重心分解を作ります。store_distances=trueなら元の木での距離情報も保存します。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** 空の構築・ムーブはO(1)、コピーは保持領域に比例。木からはO(N log(N+1))時間、保持領域は距離ありO(N log(N+1))、距離なしO(N)。

### build

```text
void build(const Graph& graph, bool store_distances = true);
```

分解を構築／再構築する。入力を変更・保持しない。

**制約:** 入力グラフ・関数・比較器の前提は下記の共通条件に従います。

**計算量:** O(N log(N+1))時間。距離ありは同じ保持領域、距離なしはO(N)保持領域。

### root

```text
int root() const noexcept;
```

根の頂点番号を返します。空なら-1です。

**制約:** なし。

**計算量:** O(1)。

### parent

```text
int parent(int vertex) const;
```

親の頂点番号を返します。根の親は-1です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### depth

```text
int depth(int vertex) const;
```

根からの深さを返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### component_size

```text
int component_size(int vertex) const;
```

vを重心として除去した時点の成分サイズを返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### ancestors

```text
const std::vector<Ancestor>& ancestors(int vertex) const;
```

重心木の根からvまでのAncestor列をconst参照で返します。最後の要素は{v,0}です。

**制約:** 距離情報を保存して構築済みで、頂点番号が有効であること。

**計算量:** O(1)。

### distance

```text
std::int64_t distance(int u, int v) const;
```

元の木の辺数距離をstd::int64_tで返す。

**制約:** 距離情報を保存して構築済みで、頂点番号が有効であること。

**計算量:** O(log(N+1))。

### parents

```text
const std::vector<int>& parents() const noexcept;
```

親番号の配列を読み取り専用で返します。

**制約:** なし。

**計算量:** O(1)。

### depths

```text
const std::vector<int>& depths() const noexcept;
```

深さの配列を読み取り専用で返します。

**制約:** なし。

**計算量:** O(1)。

### order

```text
const std::vector<int>& order() const noexcept;
```

構築時に得た頂点の順序を読み取り専用で返します。

**制約:** なし。

**計算量:** O(1)。

### centroid_tree

```text
Graph centroid_tree() const;
```

重心木の無向隣接リストを独立した値として返します。

**制約:** なし。

**計算量:** O(N)時間・領域。

### size

```text
std::size_t size() const noexcept;
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

### stores_distances

```text
bool stores_distances() const noexcept;
```

元の木での距離情報を保持していればtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### clear

```text
void clear() noexcept;
```

領域を解放して空の状態へ戻します。

**制約:** なし。

**計算量:** 保持領域の解放に比例します。

### swap

```text
void swap(CentroidDecomposition& other) noexcept;
void swap(CentroidDecomposition& a, CentroidDecomposition& b) noexcept;
```

保持するグラフ・結果・状態を交換します。

**制約:** なし。

**計算量:** O(1)。


```cpp
Graph graph{{1}, {0,2}, {1,3}, {2}};
CentroidDecomposition decomposition(graph);
auto distance = decomposition.distance(0,3); // 3。
for (auto a : decomposition.ancestors(3)) {
    // a.vertexは祖先の重心、a.distanceは元の木での3からの距離。
}
```

## 距離情報の使い方

Ancestorは{vertex,distance}です。距離には**元の木での辺数**を使い、各祖先列の長さはO(log(N+1))です。印の追加や最寄りの印の照会に使う場合は、完全な使用例を参照してください。

既定では距離情報も構築し、O(N log(N+1))時間・領域です。`CentroidDecomposition(graph,false)`または`build(graph,false)`なら距離を保存せず、領域O(N)です。この場合ancestors・distanceは使えません。distanceはO(log(N+1))、centroid_treeはO(N)、結果配列の参照・単点取得はO(1)です。

## 入力・所有

頂点0..N-1の単純無向木が前提です。空の木も許します。入力は構築後に破棄・変更できます。

コピーは独立、move元は空、swapはO(1)です。コピー代入・buildに失敗した場合は旧結果を維持します。結果への参照は再構築・代入・clear・破棄まで有効です。

既定では初回の成分探索に、頂点番号・循環・到達頂点数・逆向きの辺の検査を組み込みます。検査専用の全辺走査や、後の成分での繰り返し検査はしません。`SKIP_RUNTIME_CHECKS`でこれらと頂点・距離保存の検査を除き、頂点数の表現上限は維持します。

数学的参考は[Nyaan](https://nyaannyaan.github.io/library/tree/centroid-decomposition.hpp.html)、機能上の参考は[Luzhiled](https://ei1333.github.io/library/graph/tree/centroid-decomposition.hpp.html)です。入力を保持しない反復構築として独立に記述しています。

## 構築・状態と補助型

### CentroidDecomposition::Ancestor::operator==

```text
bool operator==(const Ancestor&, const Ancestor&);
```

祖先頂点と距離の両フィールドが一致すればtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### operator=

```text
CentroidDecomposition& operator=(const CentroidDecomposition& other);
CentroidDecomposition& operator=(CentroidDecomposition&& other) noexcept;
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用を加えます。
