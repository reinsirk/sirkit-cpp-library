# Fenwick Tree

`fenwick_tree<T>`は、点への加算と区間和を扱います。添字は0始まり、区間は左端を含み右端を含まない[l,r)です。公開名・基本操作はACL形式で、`atcoder::`は付けません。[使用例](../Examples/fenwick_tree_example.cpp)を参照してください。

以下、Nは要素数です。値の演算・コピーをO(1)として計算量を記します。

## コンストラクタ

```text
fenwick_tree();
explicit fenwick_tree(int n);
explicit fenwick_tree(const std::vector<T>& values);
```

空列、0をn個並べた列、または指定vectorから構築します。

**制約:** 冒頭の値型・演算の前提を満たすこと。指定する列長は内部容量の上限内です。

**計算量:** N要素にO(N+1)時間・領域。 ムーブ構築はO(1)、コピーは保持領域に比例。

## add

```text
void add(int i, T delta);
```

添字iの要素へdeltaを加算します。

**制約:** 指定位置は0以上size()未満。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## sum

```text
T sum(int l, int r);
```

半開区間[l,r)の和をTで返します。空区間はT{}です。

**制約:** 0<=l<=r<=size()。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## get

```text
T get(int i) const;
```

指定位置の値を取得します。

**制約:** 指定位置は0以上size()未満。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## set

```text
void set(int i, T value);
```

指定位置の値を置き換えます。

**制約:** 指定位置は0以上size()未満。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## lower_bound

```text
int lower_bound(const T& target) const;
```

先頭からの和が目標値以上になる最初の添字を返します。なければsize()、非正の目標値なら0です。

**制約:** 全要素が非負で、途中の和がTに収まり、加算と順序が整合すること。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## upper_bound

```text
int upper_bound(const T& target) const;
```

先頭からの和が目標値より大きくなる最初の添字を返します。なければsize()、負の目標値なら0です。

**制約:** 全要素が非負で、途中の和がTに収まり、加算と順序が整合すること。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## size

```text
int size() const noexcept;
```

要素数を返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## empty

```text
bool empty() const noexcept;
```

空ならtrueを返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## values

```text
std::vector<T> values() const;
```

現在の列をvectorへコピーして返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(N+1)時間・O(N)追加領域。

## reset

```text
void reset(int n);
void reset();
```

引数付きは0をn個並べ、省略形は現在の長さを保って全要素を0へ戻します。

**制約:** 共通の前提を満たすこと。

**計算量:** n指定形はO(旧N+新N+1)、引数省略形はO(N+1)。

## clear

```text
void clear() noexcept;
```

確保容量を保って空列にします。

**制約:** 共通の前提を満たすこと。

**計算量:** O(旧N+1)の上界。要素・設定の破棄費用を含みます。

## swap

```text
void swap(fenwick_tree& other) noexcept;
```

内容と設定を交換します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。値型・比較器のswapの費用は別です。

```cpp
fenwick_tree<std::int64_t> sums(std::vector<std::int64_t>{2,0,3,5});
sums.add(1,4);                // [2,4,3,5]。
sums.set(3,1);                // [2,4,3,1]。
auto total = sums.sum(1,4);   // 8。
auto index = sums.lower_bound(7); // 2。先頭からの和は2,6,9,10。
```

## 境界探索と数値の前提

点操作・区間和・境界探索はO(log(N+1))、領域はO(N)です。reset(n)は旧列の破棄を含めO(旧N+新N+1)、reset()はO(N+1)。演算とコピーをO(1)とします。先頭の和はsum(0,r)、全体はsum(0,size())で取得します。

lower_boundは非正の目標値なら0、upper_boundは負の目標値なら0です。探索中は全要素が非負で、途中の和がTに収まり、加算と順序が整合することが前提です。剰余で和が周回する場合は使えません。非負条件の検査だけで全列を走査しません。

bool以外の整数型は内部を同幅の符号なし型で保持し、型幅のmod 2^bitで計算します。符号付き型でも内部の符号付きオーバーフローを起こしません。利用可能な128ビット整数拡張も同様です。modintや独自型は、T{}を0とする可換な加減算を正しく定義してください。Mintは利用側で用意します。

列長はintの範囲と内部容量に収めます。不正な添字・区間は既定で例外により検査し、`SKIP_RUNTIME_CHECKS`で除きます。容量・添字の保護は維持します。sumはACLと同じ非constです。コピーは独立、移動元は空列として再利用できます。

[ACLのAPI仕様](https://atcoder.github.io/ac-library/production/document_ja/fenwicktree.html)と[Nyaanの解説](https://nyaannyaan.github.io/library/data-structure/binary-indexed-tree.hpp)を参照しています。実装は本ライブラリ用のCC0です。動的座標には[動的Fenwick Tree](dynamic_fenwick.md)を使います。

## operator=

```text
fenwick_tree& operator=(const fenwick_tree&) = default;
fenwick_tree& operator=(fenwick_tree&& other) noexcept;
```

コピーまたはムーブで内容を置き換え、自身への参照を返します。コピーは独立した内容を持ちます。

**制約:** 型・所有・例外保証は共通の説明に従います。

**計算量:** 保持する配列・ノードのコピーと旧内容の破棄に比例。ムーブ構築とは異なり、代入では旧内容の解放費用も含みます。
