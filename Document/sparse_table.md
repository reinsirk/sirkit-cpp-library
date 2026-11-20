# Sparse Table

`SparseTable<T,Op>`は、変更しない列の区間の最小値・最大値・gcdなどをO(1)で求めます。区間は0始まりの半開区間[l,r)です。[使用例](../Examples/sparse_table_example.cpp)を参照してください。

Opには結合的な演算を指定します。同じ値を重ねても結果が変わらない`op(x,x)==x`が必要で、この性質をべき等性と呼びます。和はこの条件を満たさないため、[Disjoint Sparse Table](disjoint_sparse_table.md)を使います。左右の引数を交換できる必要はありません。単位元identityには、左右どちらの引数に置いてももう一方の値を変えない値を指定します。

以下、Nは要素数です。値の演算・コピーをO(1)として計算量を記します。

## コンストラクタ

```text
explicit SparseTable(const std::vector<T>& values = {}, const T& identity = T{}, Op op = Op{});
SparseTable(const SparseTable&) = default;
SparseTable(SparseTable&& other);
```

引数を省略すると空列で構築し、valuesを渡すとその列で構築します。コピーした列は元の列から独立しています。

**制約:** 冒頭の値型・演算の前提を満たすこと。指定する列長は内部容量の上限内です。

**計算量:** N要素にO(N log(N+1)+1)時間・領域。 ムーブ構築はO(1)、コピーは保持領域に比例。

## get

```text
const_reference get(std::size_t i) const;
```

元の要素のconst参照を返します。T=boolでは値を返します。

**制約:** 指定位置は0以上size()未満。

**計算量:** O(1)。

## prod

```text
T prod(std::size_t l, std::size_t r) const;
```

半開区間[l,r)を左から集約した値を返します。空区間は単位元です。

**制約:** 区間は0<=l<=r<=size()。点を指定する形は0<=p<size()。

**計算量:** O(1)。

## all_prod

```text
T all_prod() const;
```

列全体を先頭から集約した値を返します。空列は単位元です。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## max_right

```text
template<class Pred> std::size_t max_right(std::size_t l, Pred pred) const;
```

左端lを固定し、述語を満たす最大の右端を返します。

**制約:** 端点は[0,size()]内。pred(単位元)=trueで、区間を伸ばしてfalseの後はtrueへ戻らないこと。

**計算量:** O(log(N+1))。述語の費用は別です。

## min_left

```text
template<class Pred> std::size_t min_left(std::size_t r, Pred pred) const;
```

右端rを固定し、述語を満たす最小の左端を返します。

**制約:** 端点は[0,size()]内。pred(単位元)=trueで、区間を伸ばしてfalseの後はtrueへ戻らないこと。

**計算量:** O(log(N+1))。述語の費用は別です。

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

## values

```text
std::vector<T> values() const;
```

元の列をvectorへコピーして返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(N+1)時間・O(N)追加領域。

## assign

```text
void assign(const std::vector<T>& values);
```

指定した列で再構築します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(旧N log(旧N+1)+新N log(新N+1)+1)時間・新列分の領域。

## clear

```text
void clear() noexcept;
```

空の状態へ戻します。容量や資源の扱いは末尾の所有・例外の説明に従います。

**制約:** 共通の前提を満たすこと。

**計算量:** 旧列の保持領域の破棄に比例。

## swap

```text
void swap(SparseTable& other);
```

内容と設定を交換します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。値型・比較器のswapの費用は別です。

```cpp
struct Minimum { int operator()(int a,int b) const { return std::min(a,b); } };
SparseTable<int,Minimum> table(
    std::vector<int>{4,2,7,1,8},std::numeric_limits<int>::max());
auto minimum = table.prod(1,4); // 1。
```

## 前提と計算量

演算・コピーをO(1)として、構築はO(N log(N+1)+1)時間・領域です。再構築は旧領域の破棄も含めO(旧N log(旧N+1)+新N log(新N+1)+1)です。

境界探索の述語は同じ値に同じ結果を返し、pred(identity)がtrueで、区間を伸ばしてfalseになった後はtrueへ戻らないことが前提です。単調性を確認する全要素走査は行いません。

Opはconst呼出可能な型を使います。添字・区間・述語の単位元条件は既定で検査し、`SKIP_RUNTIME_CHECKS`で除きます。参照は再構築・clear・移動・交換後まで保持しないでください。コピーは独立、移動元は空として再利用できます。演算等の例外時に内容の強い保証は行いません。

32bit以下の整数型（bool以外）では、条件が合えば対応するGCC／Clang・CPUでAVX2を使って構築します。`SIRKIT_DISABLE_SIMD`で専用経路とCPU判定を除外できます。非対応環境やその他の値型は標準C++の経路で処理します。

数学的参考は[AtCoder Algorithm Lectures](https://info.atcoder.jp/entry/algorithm_lectures/sparse_table)です。実装は本ライブラリ用のCC0です。

## operator=

```text
SparseTable& operator=(const SparseTable& other);
SparseTable& operator=(SparseTable&& other);
```

コピーまたはムーブで内容を置き換え、自身への参照を返します。コピーは独立した内容を持ちます。

**制約:** 型・所有・例外保証は共通の説明に従います。

**計算量:** 保持する配列・ノードのコピーと旧内容の破棄に比例。ムーブ構築とは異なり、代入では旧内容の解放費用も含みます。
