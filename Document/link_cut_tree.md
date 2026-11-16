# Link Cut Tree

`LinkCutTree<T,Op>`は、辺を追加・削除する森で頂点の値とパスの和などを管理します。頂点番号は0始まりで、番号の大小に親子関係の意味はありません。構築直後は各頂点が孤立し、辺はlinkで追加します。[基本例](../Examples/link_cut_tree_example.cpp)・[非可換合成](../Examples/link_cut_tree_composite_example.cpp)・[遅延作用](../Examples/link_cut_tree_lazy_example.cpp)を参照してください。

以下、Nは要素数です。値の演算・コピーをO(1)として計算量を記します。

## コンストラクタ

```text
explicit LinkCutTree(std::span<const T> values = {}, const T& identity = T{}, Op op = Op{}, Mapping mapping = Mapping{}, Composition composition = Composition{});
explicit LinkCutTree(const std::vector<T>& values, const T& identity = T{}, Op op = Op{}, Mapping mapping = Mapping{}, Composition composition = Composition{});
LinkCutTree(const LinkCutTree& other);
LinkCutTree(LinkCutTree&& other) noexcept;
```

入力値を各頂点へ置き、辺のない森を構築します。span形の引数省略は空の森です。

**制約:** 冒頭の値型・演算の前提を満たすこと。指定する列長は内部容量の上限内です。

**計算量:** N要素にO(N+1)時間・領域。 ムーブ構築はO(1)、コピーは保持領域に比例。

## link

```text
bool link(int first, int last);
```

異なる木の2頂点を辺で結びtrueを返します。既に連結ならfalseです。

**制約:** 指定する頂点は0<=v<size()。演算中の例外後はusable()を確認すること。

**計算量:** 1回の操作は償却O(log(N+1))。演算・値のコピーをO(1)とします。

## cut

```text
bool cut(int first, int last);
```

2頂点間の直接の辺を削除してtrueを返します。辺がなければfalseです。

**制約:** 指定する頂点は0<=v<size()。演算中の例外後はusable()を確認すること。

**計算量:** 1回の操作は償却O(log(N+1))。演算・値のコピーをO(1)とします。

## connected

```text
bool connected(int first, int last);
```

2頂点が同じ木にあればtrueを返します。

**制約:** 指定する頂点は0<=v<size()。演算中の例外後はusable()を確認すること。

**計算量:** 1回の操作は償却O(log(N+1))。演算・値のコピーをO(1)とします。

## get

```text
T get(int vertex);
```

頂点の現在値を返します。

**制約:** 指定する頂点は0<=v<size()。演算中の例外後はusable()を確認すること。

**計算量:** 1回の操作は償却O(log(N+1))。演算・値のコピーをO(1)とします。

## set

```text
void set(int vertex, const T& value);
```

頂点の値をvalueへ置き換えます。

**制約:** 指定する頂点は0<=v<size()。演算中の例外後はusable()を確認すること。

**計算量:** 1回の操作は償却O(log(N+1))。演算・値のコピーをO(1)とします。

## transform

```text
template<class Fn> void transform(int vertex, Fn&& function);
```

頂点の現在値へ関数を適用して置き換えます。

**制約:** 指定する頂点は0<=v<size()。演算中の例外後はusable()を確認すること。

**計算量:** 1回の操作は償却O(log(N+1))。演算・値のコピーをO(1)とします。

## path_prod

```text
std::optional<T> path_prod(int first, int last);
```

first→lastの頂点値を順に集約したoptional<T>を返します。非連結ならnulloptです。

**制約:** 指定する頂点は0<=v<size()。演算中の例外後はusable()を確認すること。

**計算量:** 1回の操作は償却O(log(N+1))。演算・値のコピーをO(1)とします。

## path_size

```text
std::size_t path_size(int first, int last);
```

経路の頂点数を返します。非連結なら0です。

**制約:** 指定する頂点は0<=v<size()。演算中の例外後はusable()を確認すること。

**計算量:** 1回の操作は償却O(log(N+1))。演算・値のコピーをO(1)とします。

## path_length

```text
std::optional<std::size_t> path_length(int first, int last);
```

経路の辺数をoptionalで返します。非連結ならnulloptです。

**制約:** 指定する頂点は0<=v<size()。演算中の例外後はusable()を確認すること。

**計算量:** 1回の操作は償却O(log(N+1))。演算・値のコピーをO(1)とします。

## kth_on_path

```text
std::optional<int> kth_on_path(int first, int last, std::size_t k);
```

first→lastの0始まりk番目の頂点をoptionalで返します。非連結・範囲外ならnulloptです。

**制約:** 指定する頂点は0<=v<size()。演算中の例外後はusable()を確認すること。

**計算量:** 1回の操作は償却O(log(N+1))。演算・値のコピーをO(1)とします。

## make_root

```text
void make_root(int vertex);
```

指定頂点を所属する木の根にします。

**制約:** 指定する頂点は0<=v<size()。演算中の例外後はusable()を確認すること。

**計算量:** 1回の操作は償却O(log(N+1))。演算・値のコピーをO(1)とします。

## find_root

```text
int find_root(int vertex);
```

所属する木の現在の根を返します。

**制約:** 指定する頂点は0<=v<size()。演算中の例外後はusable()を確認すること。

**計算量:** 1回の操作は償却O(log(N+1))。演算・値のコピーをO(1)とします。

## lca

```text
std::optional<int> lca(int root, int first, int last);
```

rootを根とみなした2頂点のLCAをoptionalで返します。3頂点が非連結ならnulloptです。

**制約:** 指定する頂点は0<=v<size()。演算中の例外後はusable()を確認すること。

**計算量:** 1回の操作は償却O(log(N+1))。演算・値のコピーをO(1)とします。

## apply

```text
void apply(int vertex, const Action& action) requires(has_action);
bool apply(int first, int last, const Action& action) requires(has_action);
```

頂点または経路へ作用を適用します。経路版は非連結ならfalse、適用時trueです。

**制約:** 作用を設定した型だけで使用可。有効な頂点番号を渡すこと。

**計算量:** 1回の操作は償却O(log(N+1))。演算・値のコピーをO(1)とします。

## size

```text
std::size_t size() const noexcept;
```

要素数を返します。

**制約:** 共通の型・演算条件を満たすこと。reset・clearは無効状態からの復旧にも使えます。

**計算量:** O(1)。

## empty

```text
bool empty() const noexcept;
```

空ならtrueを返します。

**制約:** 共通の型・演算条件を満たすこと。reset・clearは無効状態からの復旧にも使えます。

**計算量:** O(1)。

## usable

```text
bool usable() const noexcept;
```

演算中の例外によって無効化されていなければtrueを返します。

**制約:** 共通の型・演算条件を満たすこと。reset・clearは無効状態からの復旧にも使えます。

**計算量:** O(1)。

## reset

```text
void reset(std::span<const T> values);
void reset(const std::vector<T>& values);
```

現在の演算設定で値を取り込み、全頂点が孤立した森を作り直します。

**制約:** 共通の型・演算条件を満たすこと。reset・clearは無効状態からの復旧にも使えます。

**計算量:** O(旧N+新N+1)時間・新列の保持領域。

## clear

```text
void clear() noexcept;
```

空の森にして無効状態も復旧します。

**制約:** 共通の型・演算条件を満たすこと。reset・clearは無効状態からの復旧にも使えます。

**計算量:** O(旧N+1)の上界。要素・設定の破棄費用を含みます。

## swap

```text
void swap(LinkCutTree& other) noexcept;
friend void swap(LinkCutTree& first, LinkCutTree& second) noexcept;
```

内容と設定を交換します。

**制約:** 共通の型・演算条件を満たすこと。reset・clearは無効状態からの復旧にも使えます。

**計算量:** O(1)。値型・比較器のswapの費用は別です。

```cpp
LinkCutTree<std::int64_t> tree(std::vector<std::int64_t>{2,4,1,7});
tree.link(0,1); tree.link(1,2); tree.link(1,3);
auto sum = tree.path_prod(2,3); // optionalに12が入る。1+4+7。
tree.cut(1,3);
auto missing = tree.path_prod(2,3); // nullopt。
```

同じ頂点へのパスは、その頂点の値・頂点数1・辺数0です。パス操作・link・cutは根の向きを変える場合があります。根が重要ならmake_rootまたはlcaの引数で明示します。取得も補助木を変更するため非constです。

## 演算とパス更新

Opはconst呼出可能な結合演算、identityは左右の単位元です。左右の順序を保ち、写像合成などの非可換な集約も扱えます。T・Actionはcopyable、設定はconst呼出可能・move_constructibleで、破棄が例外を投げない型を使います。

パス更新には`LinkCutTree<T,Op,Action,Mapping,Composition>`を指定します。Mappingは`mapping(f,集約値,頂点数)`で更新後の集約を返し、Compositionは`composition(newer,older)`でolderの後にnewerを適用する更新を返します。更新は集約へ分配でき、反転と可換であることが前提です。作用なしの型は更新用の状態を持ちません。

構築はO(N+1)時間・領域、森操作は償却O(log(N+1))、1回は最悪O(N+1)です。値・更新のコピーや演算をO(1)とします。数値は途中計算も値域内に収め、コールバックから同じ木へ再入しないでください。

## 所有権と検査

コピーは頂点・辺・遅延の領域が独立し、演算設定はconstで共有します。移動はnoexcept・O(1)、移動元は同じ設定の空の森として再利用できます。コピー代入・resetは確保やコピーの失敗時に元を保ちます。resetはO(旧N+新N+1)、clearはO(N+1)です。

森操作中に値・演算・更新等が例外を投げると、`usable()==false`になります。reset・clear・代入で復旧するまで森操作はlogic_errorを投げます。失敗状態をコピー・移動した先も復旧が必要です。演算の外部状態は巻き戻しません。

頂点番号は既定で検査し、`SKIP_RUNTIME_CHECKS`で除きます。N<=INT_MAXの容量保護、接続・辺・順位による戻り値、例外後の無効化は常に維持します。番号の入力検査エラーでは無効化しません。

数学的参考は[Nyaan](https://nyaannyaan.github.io/library/lct/link-cut-base.hpp)と[Luzhiled](https://ei1333.github.io/library/structure/dynamic-tree/lazy-link-cut-tree.hpp)です。実装は独立に組み立て、内部配置と検証結果は作業記録に置きます。

## operator=

```text
LinkCutTree& operator=(const LinkCutTree& other);
LinkCutTree& operator=(LinkCutTree&& other) noexcept;
```

コピーまたはムーブで内容を置き換え、自身への参照を返します。コピーは独立した内容を持ちます。

**制約:** 型・所有・例外保証は共通の説明に従います。

**計算量:** 保持する配列・ノードのコピーと旧内容の破棄に比例。ムーブ構築とは異なり、代入では旧内容の解放費用も含みます。
