# 双対セグメント木

`DualSegmentTree<T,Action,Mapping,Composition>`は、区間をまとめて更新し、1要素の値を取得します。区間和などの集約が不要な場合に使います。添字は0始まり、区間は半開区間[l,r)です。[使用例](../Examples/dual_segment_tree_example.cpp)を参照してください。

Mappingは`mapping(f,x)`で更新後の値、Compositionは`composition(newer,older)`で「olderの後にnewer」を実行する更新を返す関数オブジェクトです。更新の合成は結合的で、Mappingと整合するように定義します。更新の順序は保持し、Actionの単位元・比較・既定構築は要求しません。

以下、Nは要素数です。値の演算・コピーをO(1)として計算量を記します。

## コンストラクタ

```text
explicit DualSegmentTree(const std::vector<T>& values,Mapping mapping,Composition composition);
explicit DualSegmentTree(std::span<const T> values,Mapping mapping,Composition composition);
DualSegmentTree(std::size_t n,const T& value,Mapping mapping,Composition composition);
DualSegmentTree(const DualSegmentTree&)=default;
DualSegmentTree(DualSegmentTree&& other) noexcept;
```

vector・span、または同じ値n個を取り込み、指定した写像・合成を設定します。

**制約:** 冒頭の値型・演算の前提を満たすこと。指定する列長は内部容量の上限内です。

**計算量:** N要素にO(N+1)時間・領域。 ムーブ構築はO(1)、コピーは保持領域に比例。

## apply

```text
void apply(std::size_t l,std::size_t r,const Action& f);
void apply(std::size_t p,const Action& f);
```

1要素または半開区間[l,r)へ更新fを適用します。空区間には何もしません。

**制約:** 区間は0<=l<=r<=size()。点を指定する形は0<=p<size()。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## get

```text
T get(std::size_t p);
```

保留中の更新を反映した値をコピーして返します。

**制約:** 指定位置は0以上size()未満。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## operator[]

```text
T operator[](std::size_t p);
```

getと同じ現在値をコピーして返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## set

```text
void set(std::size_t p,const T& value);
```

指定位置の値を置き換えます。

**制約:** 指定位置は0以上size()未満。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## transform

```text
template<class Fn> void transform(std::size_t p,Fn&& fn);
```

指定位置の現在値を関数へ渡し、戻り値で置き換えます。

**制約:** 指定位置は0以上size()未満。

**計算量:** O(log(N+1))。値の演算・コピーをO(1)とします。

## values

```text
std::vector<T> values();
```

現在の列をvectorへコピーして返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(N+1)時間・O(N)追加領域。

## size

```text
std::size_t size() const noexcept;
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

## usable

```text
bool usable() const noexcept;
```

演算中の例外によって無効化されていなければtrueを返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## reset

```text
void reset(const std::vector<T>& values);
void reset(std::span<const T> values);
void reset(std::size_t n,const T& value);
```

vector・spanまたは同じ値n個から、現在の演算設定のまま作り直します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(旧N+新N+1)時間・新列の保持領域。

## clear

```text
void clear() noexcept;
```

空列にして配列を解放し、例外で無効になった状態も復旧します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(旧N+1)の上界。要素・設定の破棄費用を含みます。

## swap

```text
void swap(DualSegmentTree& other) noexcept;
friend void swap(DualSegmentTree& a,DualSegmentTree& b) noexcept;
```

内容と設定を交換します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。値型・比較器のswapの費用は別です。

```cpp
struct Add {
    std::int64_t operator()(std::int64_t f,std::int64_t x) const { return x+f; }
};
DualSegmentTree<std::int64_t,std::int64_t,Add,std::plus<std::int64_t>> tree(
    std::vector<std::int64_t>{1,2,3},Add{},std::plus<std::int64_t>{});
tree.apply(0,2,4); // [5,6,3]。
auto value = tree.get(1); // 6。
```

## 前提・所有権・例外

演算・コピーをO(1)として、構築はO(N+1)時間・領域、更新・点操作はO(log(N+1))です。取得も保留中の更新を伝えるため非constです。T・Actionはコピー可能で、設定はconst呼出可能・move_constructibleにします。数値演算は途中計算も値域内に収め、コールバックから同じ木へ再入しないでください。

コピーの値・更新は独立し、演算設定はconstで共有します。移動はnoexcept・O(1)、移動元は同じ設定の空列として再利用できます。コピー代入・resetは確保やコピーの失敗時に元を保ちます。

操作中に値・更新・写像等が例外を投げると、`usable()==false`になります。以後はreset・clear・代入で復旧するまで操作がlogic_errorを投げます。値と更新には失敗後も安全に破棄できる型を使います。入力検査のエラーでは無効化せず、演算の外部状態は巻き戻しません。

点・区間の検査は既定で有効です。`SKIP_RUNTIME_CHECKS`は入力検査だけを除き、容量・添字の保護と例外後の無効化は維持します。

数学的構成と問題選定は[Luzhiledの双対セグメント木](https://ei1333.github.io/library/structure/segment-tree/dual-segment-tree.hpp.html)を参考にし、実装は独立に組み立てています。

## operator=

```text
DualSegmentTree& operator=(const DualSegmentTree& other);
DualSegmentTree& operator=(DualSegmentTree&& other) noexcept;
```

コピーまたはムーブで内容を置き換え、自身への参照を返します。コピーは独立した内容を持ちます。

**制約:** 型・所有・例外保証は共通の説明に従います。

**計算量:** 保持する配列・ノードのコピーと旧内容の破棄に比例。ムーブ構築とは異なり、代入では旧内容の解放費用も含みます。
