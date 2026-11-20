# 重軽分解

`HeavyLightDecomposition`は無向森を分解し、経路・部分木をDFS添字の半開区間へ変換します。`with_values<Range>(values)`は分解と集計器を所有する`HeavyLightValues`を返し、頂点番号で更新・集計できます。値は`values[v]`が頂点vとなる順に渡します。

BIT／Segtreeは別途貼り付けます。`Range`は`fenwick_tree<T>`・`segtree<S,op,e>`・`lazy_segtree<S,op,e,F,mapping,composition,id>`、または実ACLの対応型です。分解だけならHLD単独で使えます。[本体](../Graph/Heavy_Light_Decomposition.cpp)・[完全な使用例](../Examples/heavy_light_decomposition_example.cpp)。

```cpp
Graph tree{{1}, {0,2}, {1}};
auto hld = HeavyLightDecomposition(tree)
    .with_values<fenwick_tree<std::int64_t>>(std::vector<std::int64_t>{10,20,30});
hld.add(1, 5);
auto sum = hld.path_prod(0, 2);      // 65。
auto subtree = hld.subtree_prod(1); // 55。
```

## 値を所有するAPI

以下、H=`HeavyLightDecomposition`、V=`HeavyLightValues<Range,Ordered>`、N=頂点数、M=辺数です。value_typeはBITならT、Segtree／LazyならS。集計器の型引数・作用の約束は[Segtree](segment_tree.md)・[Lazy Segtree](lazy_segment_tree.md)と同じです。

`value_type`は保持する値・集計結果の型です。

### HeavyLightValues（構築）

```text
HeavyLightValues();
HeavyLightValues(const Graph& graph,const std::vector<value_type>& values,int root=0);
HeavyLightValues(const HeavyLightDecomposition& source,const std::vector<value_type>& values);
HeavyLightValues(HeavyLightDecomposition&& source,const std::vector<value_type>& values);
HeavyLightValues(const HeavyLightValues& other);
HeavyLightValues(HeavyLightValues&&);
```

空の状態、または分解と頂点順の値から構築します。既存の分解はコピー／移動で取り込み、コピー構築は独立した状態を作ります。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** 空の構築・ムーブはO(1)。GraphからはO(N+M)＋集計器構築。既存分解からは並べ替えO(N)＋集計器構築、分解をコピーする版はさらにO(N)。コピー構築は保持領域に比例します。

### V::operator=

```text
HeavyLightValues& operator=(HeavyLightValues other) noexcept;
```

コピーまたはムーブで代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用も加わります。

### V::build

```text
void build(const Graph& graph,const std::vector<value_type>& values,int root=0);
```

分解と値を一緒に再構築。戻り値void。

**制約:** 値の列は長さN。グラフは対称な無向森で、集計器は下記の対応条件を満たすこと。

**計算量:** O(N+M)＋集計器構築。旧領域の破棄費用を加えます。

### V::clear

```text
void clear() noexcept;
```

領域を解放して空の状態へ戻します。

**制約:** なし。

**計算量:** 保持領域の解放に比例します。

### V::swap

```text
void swap(HeavyLightValues& other) noexcept;
```

保持するグラフ・結果・状態を交換します。

**制約:** なし。

**計算量:** O(1)。

### V::get

```text
value_type get(int v);
```

指定頂点の値をvalue_typeで返します。

**制約:** 頂点番号が有効で、使う操作に対応した集計器を用意すること。

**計算量:** SegtreeでO(1)、他の対応集計器でO(log N)。

### V::set

```text
void set(int v,const value_type x);
```

指定頂点の値を置き換えます。

**制約:** 頂点番号が有効で、使う操作に対応した集計器を用意すること。

**計算量:** 標準の対応集計器でO(log N)。

### V::add

```text
void add(int v,const value_type x);
```

指定頂点の値へ加算します。

**制約:** 頂点番号が有効で、使う操作に対応した集計器を用意すること。

**計算量:** 標準の対応集計器でO(log N)。

### V::apply

```text
template<class F> void apply(int v,const F f);
```

指定頂点の値へ作用を適用します。

**制約:** 頂点番号が有効で、使う操作に対応した集計器を用意すること。

**計算量:** 標準の対応集計器でO(log N)。

### V::path_prod

```text
value_type path_prod(int u,int v,bool edges=false);
```

u→vの通行順に経路を集約して返します。

**制約:** 両端は同じ木の有効な頂点番号です。集計器・通行順・辺モードの前提はこの節の共通条件に従います。

**計算量:** 標準のBIT・Segtree・Lazy SegtreeでO(log²N)。

### V::subtree_prod

```text
value_type subtree_prod(int v,bool edges=false);
```

部分木をDFS添字昇順に集約して返します。

**制約:** 頂点番号が有効で、使う操作に対応した集計器を用意すること。

**計算量:** 標準のBIT・Segtree・Lazy SegtreeでO(log N)。

### V::path_apply

```text
template<class F> void path_apply(int u,int v,const F f,bool edges=false);
```

経路の各区間へ指定の作用を適用します。

**制約:** 両端は同じ木の有効な頂点番号です。集計器・通行順・辺モードの前提はこの節の共通条件に従います。

**計算量:** 標準のLazy SegtreeでO(log²N)。

### V::subtree_apply

```text
template<class F> void subtree_apply(int v,const F f,bool edges=false);
```

部分木の区間へ指定の作用を適用します。

**制約:** 頂点番号が有効で、使う操作に対応した集計器を用意すること。

**計算量:** 標準のLazy SegtreeでO(log N)。

### V::values

```text
std::vector<value_type> values();
```

全値を頂点番号順のvector<value_type>で返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** Range.values()の費用＋O(N)。values()がなければN回のgetの費用です。


値操作は非constです。`get/set/add`はBIT、`get/set`はSegtree、`get/set/apply`はLazy等の対応する集計器で使えます。Vからも以下の分解照会を呼べますが、Vに`with_values`や外部集計器用オーバーロードはありません。

`Ordered=false`は和・最小値など可換な演算用です。文字列連結・関数合成では`true`を指定し、経路u→vの順に結合します。集計器2本分を使い、値の順だけを反転します。個々の値を逆元へ変換する機能ではありません。部分木積の順はDFS添字昇順です。

辺の値は親辺を子頂点の位置へ置き、各根に単位元を置いて`edges=true`を指定します。経路はLCAの位置、部分木は指定頂点の位置を除きます。`path_prod(v,v,true)`や葉の辺部分木は単位元、空区間への更新は無操作です。

## 分解の構築

### HeavyLightDecomposition（構築）

```text
HeavyLightDecomposition();
explicit HeavyLightDecomposition(const Graph& graph,int root=0);
HeavyLightDecomposition(const HeavyLightDecomposition&);
HeavyLightDecomposition(HeavyLightDecomposition&& other) noexcept;
```

空の状態、または無向森から分解を構築します。コピーは独立し、ムーブ元は空になります。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** 空の構築はO(1)、分解はO(N+M)時間・O(N)領域。旧状態の破棄費用を加えます。

### H::operator=

```text
HeavyLightDecomposition& operator=(HeavyLightDecomposition other) noexcept;
```

コピーまたはムーブで代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用も加わります。

### H::build

```text
void build(const Graph& graph,int root=0);
```

再構築／空にする／交換。戻り値void。

**制約:** 値の列は長さN。グラフは対称な無向森で、集計器は下記の対応条件を満たすこと。

**計算量:** 空の構築はO(1)、分解はO(N+M)時間・O(N)領域。旧状態の破棄費用を加えます。

### H::clear

```text
void clear() noexcept;
```

領域を解放して空の状態へ戻します。

**制約:** なし。

**計算量:** 保持領域の解放に比例します。

### H::swap

```text
void swap(HeavyLightDecomposition& other) noexcept;
```

保持するグラフ・結果・状態を交換します。

**制約:** なし。

**計算量:** O(1)。

### H::with_values

```text
template<class Range,bool Ordered=false,class T> HeavyLightValues<Range,Ordered> with_values(const std::vector<T>& values) const &;
template<class Range,bool Ordered=false,class T> HeavyLightValues<Range,Ordered> with_values(const std::vector<T>& values) &&;
```

長さNの頂点順vectorを受け、分解をコピーしてVを返す。 同じ入力から、分解を移動してVを返す。

**制約:** 値の列は長さN。グラフは対称な無向森で、集計器は下記の対応条件を満たすこと。

**計算量:** 並べ替えO(N)＋集計器構築。const&版は分解のコピーO(N)も加わります。


Graphは`std::vector<std::vector<int>>`です。`with_values`は値配列への参照を保持しません。コピー／move代入・buildは各操作の費用に旧状態の破棄を加えます。

## 分解結果の取得（H・V共通）

全てconstで呼べます。頂点・DFS添字はint、祖先・jumpのkはstd::int64_tです。`H::Interval`は`{int left,right; bool reversed;}`。区間は頂点番号ではなくDFS添字で、reversedならright−1からleftへ読みます。VにIntervalの型別名はありません。

### H::size

```text
int size() const noexcept;
```

頂点数を返します。

**制約:** なし。

**計算量:** O(1)。

### H::empty

```text
bool empty() const noexcept;
```

頂点数が0ならtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### H::root

```text
int root() const noexcept;
```

根の頂点番号を返します。空なら-1です。

**制約:** なし。

**計算量:** O(1)。

### H::component_count

```text
int component_count() const noexcept;
```

分解した連結成分の数を返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### H::parent

```text
int parent(int v) const;
```

親の頂点番号を返します。根の親は-1です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### H::depth

```text
int depth(int v) const;
```

根からの深さを返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### H::component

```text
int component(int v) const;
```

所属する木の番号を0始まりで返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### H::root_of

```text
int root_of(int v) const;
```

所属する木の根の頂点番号を返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### H::index

```text
int index(int v) const;
```

頂点番号をDFS添字へ変換します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### H::vertex_at

```text
int vertex_at(int i) const;
```

DFS添字を頂点番号へ変換します。

**制約:** 0<=i<N。

**計算量:** O(1)。

### H::order

```text
const std::vector<int>& order() const noexcept;
```

添字iにvertex_at(i)を置くvector<int>のconst参照を返します。

**制約:** なし。

**計算量:** O(1)。

### H::reorder

```text
template<class T> std::vector<T> reorder(const std::vector<T>& values) const;
```

頂点順の値をDFS添字順へコピーして返します。

**制約:** 値の列は長さN。グラフは対称な無向森で、集計器は下記の対応条件を満たすこと。

**計算量:** O(N)時間・追加領域。

### H::restore

```text
template<class T> std::vector<T> restore(const std::vector<T>& values) const;
```

DFS添字順の値を頂点順へコピーして返します。

**制約:** 値の列は長さN。グラフは対称な無向森で、集計器は下記の対応条件を満たすこと。

**計算量:** O(N)時間・追加領域。

### H::same_component

```text
bool same_component(int u,int v) const;
```

2頂点が同じ木に属すならtrueを返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### H::is_ancestor

```text
bool is_ancestor(int u,int v) const;
```

uがvの祖先ならtrueを返します。自身は祖先とし、異成分ならfalseです。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### H::subtree_range

```text
std::pair<int,int> subtree_range(int v,bool edges=false) const;
```

pair<int,int>の半開区間[l,r)。edges=trueはvの位置を除く。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### H::subtree_size

```text
int subtree_size(int v) const;
```

指定頂点を含む部分木の頂点数を返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### H::lca

```text
int lca(int u,int v) const;
```

intのLCA／経路の辺数。別の木なら-1。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(log N)。pathの結果領域はO(log N)、for_each_pathにはコールバックの費用を加えます。

### H::distance

```text
int distance(int u,int v) const;
```

intのLCA／経路の辺数。別の木なら-1。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(log N)。pathの結果領域はO(log N)、for_each_pathにはコールバックの費用を加えます。

### H::kth_ancestor

```text
int kth_ancestor(int v,std::int64_t k) const;
```

k個上の頂点をintで返す。k=0はv、負数・根を超える場合は-1。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(log N)。pathの結果領域はO(log N)、for_each_pathにはコールバックの費用を加えます。

### H::jump

```text
int jump(int u,int v,std::int64_t k) const;
```

u→vの0始まりk番目の頂点をintで返す。負数・範囲外・別の木は-1。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(log N)。pathの結果領域はO(log N)、for_each_pathにはコールバックの費用を加えます。

### H::for_each_path

```text
template<class Fn> bool for_each_path(int u,int v,Fn&& fn,bool edges=false) const;
```

通行順にfn(left,right,reversed)を呼び、接続ならtrue、別の木ならfalse。別の木では呼ばず、fnの戻り値は無視する。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(log N)。pathの結果領域はO(log N)、for_each_pathにはコールバックの費用を加えます。

### H::path

```text
std::vector<Interval> path(int u,int v,bool edges=false) const;
```

同じ通行順のvector<H::Interval>を返す。別の木は空。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(log N)。pathの結果領域はO(log N)、for_each_pathにはコールバックの費用を加えます。


`for_each_path`にはfnの実行費用も加わります。辺モードでu=vならfnを呼ばずtrue、pathは空です。

```cpp
HeavyLightDecomposition layout(tree); // 集計器なし。
auto [l,r] = layout.subtree_range(1);
auto parts = layout.path(2,0);       // 通行順の区間列。
auto vertex = layout.vertex_at(l);  // DFS添字を頂点へ戻す。
```

## 外部集計器を使うAPI（Hのみ）

Hはconstでも使えます。集計器にはDFS添字順・長さNの値を置きます。Qは区間照会1回、Aは区間更新1回の費用です。

### H::path_prod

```text
template<class Identity,class Q,class Op> auto path_prod(int u,int v,Identity&& identity,Q&& query,Op&& op,bool edges=false) const;
template<class Identity,class Q,class Op,class Reverse> auto path_prod(int u,int v,Identity&& identity,Q&& query,Op&& op,Reverse&& reverse,bool edges=false) const;
template<class Range> auto path_prod(int u,int v,Range& range,bool edges=false) const;
template<template<class,auto,auto> class R,class S,auto Op,auto E> S path_prod(int u,int v,const R<S,Op,E>& range,bool edges=false) const;
template<template<class,auto,auto> class R,class S,auto Op,auto E,class Reverse> S path_prod(int u,int v,const R<S,Op,E>& range,Reverse&& reverse,bool edges=false) const;
template<template<class,auto,auto,class,auto,auto,auto> class R,class S,auto Op,auto E,class F,auto M,auto C,auto I> S path_prod(int u,int v,R<S,Op,E,F,M,C,I>& range,bool edges=false) const;
template<template<class,auto,auto,class,auto,auto,auto> class R,class S,auto Op,auto E,class F,auto M,auto C,auto I,class Reverse> S path_prod(int u,int v,R<S,Op,E,F,M,C,I>& range,Reverse&& reverse,bool edges=false) const;
```

u→vの通行順に経路を集約して返します。

**制約:** 両端は同じ木の有効な頂点番号です。集計器・通行順・辺モードの前提はこの節の共通条件に従います。

**計算量:** O(log N)回の区間照会・結合。照会1回Q、値コピー・結合をO(1)とするとO(Q log N)。

### H::subtree_prod

```text
template<class Q> auto subtree_prod(int v,Q&& query,bool edges=false) const;
```

部分木をDFS添字昇順に集約して返します。

**制約:** 頂点番号が有効で、使う操作に対応した集計器を用意すること。

**計算量:** 区間照会1回の費用。

### H::path_apply

```text
template<class Range,class F> void path_apply(int u,int v,Range& range,const F f,bool edges=false) const;
```

経路の各区間へ指定の作用を適用します。

**制約:** 両端は同じ木の有効な頂点番号です。集計器・通行順・辺モードの前提はこの節の共通条件に従います。

**計算量:** O(log N)回の区間更新。1回の費用をAとしてO(A log N)。

### H::subtree_apply

```text
template<class Range,class F> void subtree_apply(int v,Range& range,const F f,bool edges=false) const;
```

部分木の区間へ指定の作用を適用します。

**制約:** 頂点番号が有効で、使う操作に対応した集計器を用意すること。

**計算量:** 区間更新1回の費用。


queryはprod/sumを持つ集計器、または`(l,r)`／`(l,r,reversed)`を受ける関数です。2引数は可換専用、3引数は通行順の結果を返します。reverseは逆順の集計値への変換で、3引数queryと併用しません（std::identityは除く）。空区間は単位元です。opは結合的でidentityを左右の単位元とし、照会・値のコピー・結合の費用も加わります。集計器や関数の参照は保持しません。

## 前提・所有・失敗

入力は頂点0..N−1の対称な無向森です。指定根の木を先に分解し、残りは最小未訪問頂点を根にします。空入力はroot=0。経路集計・更新の両端は同じ木が前提です。頂点・自己辺・閉路・多重辺は探索中に検査し、隣接列の対称性は利用側で保証します。`SKIP_RUNTIME_CHECKS`で入力検査と検査専用状態を除きます。BITの整数和は型幅で周回します。

コピーは独立、move元は空です。build・コピー代入の失敗時は元を保ちますが、分解の移動を指定した構築の失敗では移動元が空になる場合があります。更新時の例外は巻き戻しません。Ordered=trueはclear/buildで作り直してください。

pathの配列は独立所有、orderは参照です。参照はclear・再構築・代入・破棄まで有効です。根や木を変えるbuildには値も渡し、辺の値は新たな親子対応へ置き直します。

並べ替えはO(N)に集計器の構築費用を加えます。実ACL FenwickはN回のaddでO(N log N)。計算量は値演算・コピーO(1)の計算量で、文字列等の費用は別です。
