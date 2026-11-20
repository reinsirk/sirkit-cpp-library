# 隣接順を保つ全方位木DP

`RerootingDP<T>`は、森の各頂点を根にした場合のDPをまとめて求めます。例えば「各頂点から全頂点までの距離和」を全ての頂点について計算できます。[本体](../Graph/Rerooting_DP.cpp)・[使用例](../Examples/rerooting_dp_example.cpp)を参照してください。

## 計算の設定

`build(identity,merge,to_vertex,to_edge)`で、辺から受け取る値と頂点の答えを同じ型Tで定義します。

- `identity`: メッセージがない場合の値。mergeの左右どちらに渡しても結果を変えない。
- `merge(const T&,const T&)->T`: 複数の隣接先から受け取った値を結合する。括り方で結果が変わらないこと。
- `to_vertex(const T&,int v)->T`: 結合した値から、頂点vを根とする答えを作る。
- `to_edge(const T&,int from,int to,size_t edge_id)->T`: 頂点の値を、fromからtoへ送る値に変える。

uからvへ送る値は、v以外からuへ届いた値を結合し、to_vertex、to_edgeの順に変換したものです。uを根とした答えは、uに隣接する全頂点から届いた値を結合してto_vertexを適用すると得られます。結合順は各頂点での**辺の追加順**なので、文字列連結のように順番が重要な演算にも使えます。

```cpp
struct State { std::int64_t sum=0,count=0; };
RerootingDP<State> tree(3);
tree.add_edge(0,1);
tree.add_edge(1,2);
tree.build(State{},
    [](const State& a,const State& b){ return State{a.sum+b.sum,a.count+b.count}; },
    [](const State& a,int){ return State{a.sum,a.count+1}; },
    [](const State& a,int,int,std::size_t){ return State{a.sum+a.count,a.count}; });
// tree[0].sum==3、tree[1].sum==2、tree[2].sum==3。
```

例のcountは頂点数、sumは距離和です。

## API

### RerootingDP

```text
explicit RerootingDP(int n = 0);
RerootingDP(const RerootingDP&);
RerootingDP(RerootingDP&& other) noexcept;
```

n頂点・辺なしの森を作ります。コピーは独立、ムーブ元は空です。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** O(N)時間・領域。コピーは保持量に比例、ムーブはO(1)。

### add_edge

```text
std::size_t add_edge(int u, int v);
```

無向辺を追加し、std::size_tの辺IDを返します。辺を追加した後に結果を取得するには、再度buildしてください。

**制約:** u,vは0..N-1。自己辺・多重辺・閉路を持たない森であること。

**計算量:** 償却O(1)。再確保時は保持要素数に比例します。

### build

```text
template<class Merge, class ToVertex, class ToEdge> std::span<const T> build(const T& identity, const Merge& merge, const ToVertex& to_vertex, const ToEdge& to_edge);
```

全頂点をそれぞれ根とするDPを計算し、答えの読み取り専用spanを返します。

**制約:** 関数・値型と森の構造は共通の前提に従います。

**計算量:** 値演算・コピーをO(1)としてO(N+M)時間・追加領域。

### values

```text
std::span<const T> values() const;
```

全頂点の答えへの読み取り専用spanを返します。

**制約:** has_results()==true。

**計算量:** O(1)。

### operator[]

```text
const T& operator[](int v) const;
```

頂点vを根とする答えへのconst参照を返します。

**制約:** 0<=v<N、has_results()==true。

**計算量:** O(1)。

### message

```text
const T& message(std::size_t id, bool reverse = false) const;
```

辺idを追加したときの向きu→vのメッセージを返します。reverse=trueなら逆向きのメッセージを返します。

**制約:** 有効な辺IDで、has_results()==true。

**計算量:** O(1)。

### messages

```text
std::span<const T> messages() const;
```

全ての有向arcのメッセージへの読み取り専用spanを返します。

**制約:** has_results()==true。

**計算量:** O(1)。

### edge

```text
std::pair<int,int> edge(std::size_t id) const;
```

指定辺の追加時の端点をpair<int,int>で返します。

**制約:** idは登録済みの辺ID。

**計算量:** O(1)。

### edges

```text
std::span<const std::pair<int,int>> edges() const noexcept;
```

追加時の端点pairを並べた読み取り専用spanを返します。

**制約:** なし。

**計算量:** O(1)。

### neighbors

```text
std::span<const std::size_t> neighbors(int v) const;
```

頂点から出るarc番号の読み取り専用spanを返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

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

### has_results

```text
bool has_results() const noexcept;
```

辺追加後にbuildを完了した有効な結果があればtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### component_count

```text
std::size_t component_count() const;
```

buildした森の連結成分数を返します。

**制約:** has_results()==true。

**計算量:** O(1)。

### reserve_edges

```text
void reserve_edges(std::size_t capacity);
```

指定件数を保持できる辺の記録容量を予約します。

**制約:** 指定件数は内部の容量上限以内です。

**計算量:** 再確保する場合は現在の保持要素数に比例します。

### reserve_neighbors

```text
void reserve_neighbors(int v, std::size_t capacity);
```

指定頂点の隣接列の容量を予約します。

**制約:** 指定件数は内部の容量上限以内です。

**計算量:** 再確保する場合は現在の保持要素数に比例します。

### reset

```text
void reset(int n = 0);
```

旧状態を破棄し、指定頂点数の空グラフへ戻します。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** 旧状態の破棄と新頂点の準備に比例します。

### clear

```text
void clear() noexcept;
```

領域を解放して空の状態へ戻します。

**制約:** なし。

**計算量:** 旧状態の破棄と新頂点の準備に比例します。

### swap

```text
void swap(RerootingDP& other) noexcept;
void swap(RerootingDP& a, RerootingDP& b) noexcept;
```

保持するグラフ・結果・状態を交換します。

**制約:** なし。

**計算量:** O(1)。


各辺idのarcは2*idが追加時のu→v、2*id+1がv→uです。結果取得はhas_results()==trueが前提で、辺を追加したら再buildします。空の森も扱い、孤立点vの答えはto_vertex(identity,v)です。

## 前提・所有と計算量

頂点は0..N-1で、自己辺・多重辺・閉路を認めません。Tはbool・cv修飾型以外でコピー・代入可能、破棄が例外を投げないことが必要です。デフォルト構築は不要です。関数はconst参照を通してbuild中だけ呼び、同じ入力に同じ結果を返すものを使います。実行中に同じグラフを変更しないでください。

値の演算・コピーをO(1)として、buildはO(N+M)時間・追加領域です。結果の参照取得はO(1)、辺追加は償却O(1)、再確保・コピーは保持量に比例します。コピーは独立、move元は空、swapはO(1)です。

build・コピー代入・resetの失敗時は旧内容を保ちます。build失敗時は旧結果の参照も維持します。辺追加に失敗した場合も辺・結果を保ちますが、辺表・隣接列の容量と以前のspanは復元しません。関数の外部状態は巻き戻しません。成功した再構築・reset・clear・代入や、対応配列の再確保で参照は無効化し得ます。

`SKIP_RUNTIME_CHECKS`で頂点・辺番号・結果の有無・森の検査を除き、内部件数・添字の上限は維持します。森の検査は通常の探索に組み込み、検査専用の全辺走査はしません。出典と検証は[記録](../Audit/Changes/graph-document-details-2026-11-15.md)を参照してください。

## 構築・状態と補助型

### operator=

```text
RerootingDP& operator=(const RerootingDP& other);
RerootingDP& operator=(RerootingDP&& other) noexcept;
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用を加えます。
