# 永続セグメント木

`PersistentSegmentTree<T,Op>`は、過去の列を残したまま点を更新します。各版をRootで指定し、古い版から分岐した更新もできます。区間は0始まりの半開区間[l,r)で、集約は左から右の順序を保ちます。[使用例](../Examples/persistent_segment_tree_example.cpp)を参照してください。

以下、Nは要素数です。値の演算・コピーをO(1)として計算量を記します。

## コンストラクタ

```text
PersistentSegmentTree(std::size_t n, Op op, T identity);
PersistentSegmentTree(std::span<const T> values, Op op, T identity);
PersistentSegmentTree(const std::vector<T>& values, Op op, T identity);
PersistentSegmentTree(const PersistentSegmentTree& other);
PersistentSegmentTree(PersistentSegmentTree&&) noexcept = default;
```

長さnの全単位元列、または指定vector・spanの初期版を構築します。

**制約:** 冒頭の値型・演算の前提を満たすこと。指定する列長は内部容量の上限内です。

**計算量:** 長さ指定はO(1)。密な列の構築はO(N+1)時間・領域。 コピーはallocated_nodes()に比例、ムーブ構築はO(1)。

## empty_root

```text
Root empty_root() const;
```

全要素がidentityの版を表すRootを返します。

**制約:** Rootの所有者と有効期間の共通条件を満たすこと。

**計算量:** O(1)。

## latest_root

```text
Root latest_root() const;
```

最後に構築・更新したRootを返します。

**制約:** Rootの所有者と有効期間の共通条件を満たすこと。

**計算量:** O(1)。

## build

```text
Root build(std::span<const T> values);
Root build(const std::vector<T>& values);
```

同じ長さの指定列から新しい版を構築し、Rootを返します。

**制約:** 入力の長さはsize()と同じ。

**計算量:** O(N+1)時間・追加領域。N>0で2N-1ノードを追加。

## set

```text
Root set(const Root& root, std::size_t position, const T& replacement);
```

指定版の一点を置き換えた新しいRootを返します。元の版は変更しません。

**制約:** Rootは現在の所有者に属し、有効期間内であること。0<=position<size()。

**計算量:** O(log(N+1))時間・追加領域。

## transform

```text
template<class F> Root transform(const Root& root, std::size_t position, F function);
```

指定版の一点へ関数を適用した新しいRootを返します。元の版を変更しません。

**制約:** Rootは現在の所有者に属し、有効期間内であること。0<=position<size()。

**計算量:** O(log(N+1))時間・追加領域。

## get

```text
T get(const Root& root, std::size_t position) const;
```

指定版の一点のTを返します。

**制約:** Rootは現在の所有者に属し、有効期間内であること。0<=position<size()。

**計算量:** O(log(N+1))。

## prod

```text
T prod(const Root& root, std::size_t left, std::size_t right) const;
```

指定版の半開区間を左から集約したTを返します。空区間はidentityです。

**制約:** 有効なRootと0<=left<=right<=size()を渡すこと。

**計算量:** O(log(N+1))。

## all_prod

```text
T all_prod(const Root& root) const;
```

指定版の全体の集約値を返します。空列はidentityです。

**制約:** 現在の所有者に属する有効なRootを渡すこと。

**計算量:** O(1)。

## max_right

```text
template<class Pred> std::size_t max_right(const Root& root, std::size_t left, Pred pred) const;
```

左端lを固定し、述語を満たす最大の右端を返します。

**制約:** 有効なRootと範囲内の端点を渡し、pred(identity)=trueかつ述語が区間の伸長に対し単調であること。

**計算量:** O(log(N+1))。

## min_left

```text
template<class Pred> std::size_t min_left(const Root& root, std::size_t right, Pred pred) const;
```

右端rを固定し、述語を満たす最小の左端を返します。

**制約:** 有効なRootと範囲内の端点を渡し、pred(identity)=trueかつ述語が区間の伸長に対し単調であること。

**計算量:** O(log(N+1))。

## size

```text
std::size_t size() const noexcept;
```

要素数を返します。

**制約:** Rootの所有者と有効期間の共通条件を満たすこと。

**計算量:** O(1)。

## empty

```text
bool empty() const noexcept;
```

空ならtrueを返します。

**制約:** Rootの所有者と有効期間の共通条件を満たすこと。

**計算量:** O(1)。

## allocated_nodes

```text
std::size_t allocated_nodes() const noexcept;
```

現在確保しているノード数を返します。

**制約:** Rootの所有者と有効期間の共通条件を満たすこと。

**計算量:** O(1)。

## reserve

```text
void reserve(std::size_t nodes);
```

追加ノードを含めて指定数まで確保できる領域を予約します。

**制約:** Rootの所有者と有効期間の共通条件を満たすこと。

**計算量:** 再確保時はallocated_nodes()に比例。予約済みならO(1)。

## reset

```text
void reset(std::size_t n);
```

列長を変更して既存ノード・版を破棄します。

**制約:** Rootの所有者と有効期間の共通条件を満たすこと。

**計算量:** 旧ノード数に比例する時間。

## clear

```text
void clear();
```

列長を0にし、既存ノード・版を破棄します。

**制約:** Rootの所有者と有効期間の共通条件を満たすこと。

**計算量:** 旧ノード数に比例する時間。

## remap_from

```text
Root remap_from(const PersistentSegmentTree& original, const Root& root) const;
```

コピー元のRootを、コピーした自分の対応するRootへ変換します。

**制約:** originalからコピーした表で、そのコピー元の有効なRootを渡すこと。

**計算量:** O(1)。

```cpp
PersistentSegmentTree<std::int64_t,std::plus<std::int64_t>> tree(
    std::vector<std::int64_t>{2,4,1},{},0);
auto first = tree.latest_root();
auto second = tree.set(first,1,9);
auto third = tree.transform(first,2,[](auto x){return x+5;});
// all_prod(first)==7、all_prod(second)==12、prod(third,1,3)==10。
```

## Rootと所有権

Rootは小さな不変の値で、作成した木へ渡します。木の破棄・reset・clear後には使えません。コピー先で元のRootを直接使うこともできません。`copy.remap_from(original,root)`で変換できるのは、直接のコピー元にコピー時点で存在したRootだけです。コピーしたノード列は独立し、以後の更新は互いに影響しません。

moveでは既存Rootが移動先へ所属し、move元には破棄・代入だけを行います。swapはRootの所属も交換します。更新・構築で確保や演算・コピーが失敗した場合、追加ノードを取り消し、古い版とlatest_rootを保持します。コピー代入には強い例外保証があります。演算の外部状態は巻き戻さず、コールバックから同じ木を再入して変更しないでください。

## 前提と計算量

Opはconst呼出可能な結合演算、identityは左右の単位元です。T・Opはコピー可能、Tの破棄は例外を投げない型を使います。演算・コピーをO(1)として、点更新はO(log(N+1))時間・追加領域、点取得・区間積・境界探索はO(log(N+1))です。密な構築はN>0で2N-1ノード、点更新は高々1+ceil(log2(N))ノードを追加します。

境界探索の述語はpred(identity)がtrueで、区間を伸ばしてfalseになった後はtrueへ戻らないことが前提です。長さ0では空のRootだけを作れ、空区間積・全体積はidentityです。reset・clearには旧ノード数に比例する破棄費用がかかります。

根の所属・添字・区間・長さは既定で検査します。`SKIP_RUNTIME_CHECKS`は入力検査と所属確認用の状態を除きますが、Rootの所属と有効期間は利用側で守る必要があります。ノード数の内部上限は常に維持します。

数学的参考は[AAL](https://info.atcoder.jp/entry/algorithm_lectures/persistent_segment_tree)、機能上の参考は[Nyaan](https://nyaannyaan.github.io/library/segment-tree/persistent-segment-tree.hpp.html)です。実装は整数添字で所有するノード列として組み直しています。

## swap

```text
void swap(PersistentSegmentTree& other) noexcept;
friend void swap(PersistentSegmentTree& a, PersistentSegmentTree& b) noexcept;
```

内容とRootの所属を交換します。

**制約:** Rootの所有者と有効期間の共通条件を満たすこと。

**計算量:** O(1)。値型・比較器のswapの費用は別です。

## operator=

```text
PersistentSegmentTree& operator=(const PersistentSegmentTree& other);
PersistentSegmentTree& operator=(PersistentSegmentTree&&) noexcept = default;
```

コピーまたはムーブで内容を置き換え、自身への参照を返します。コピーは独立した内容を持ちます。

**制約:** 型・所有・例外保証は共通の説明に従います。

**計算量:** 保持する配列・ノードのコピーと旧内容の破棄に比例。ムーブ構築とは異なり、代入では旧内容の解放費用も含みます。
