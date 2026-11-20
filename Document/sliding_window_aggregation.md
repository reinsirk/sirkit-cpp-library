# Sliding Window Aggregation

`SlidingWindowAggregation<T,Op>`は、末尾へ追加・先頭から削除するキューと、その全体の集約値を管理します。スライドする区間の和・最小値・写像合成などに使えます。Opはconst呼出可能な結合演算、identityは左右の単位元です。逆演算や左右の交換は要求しません。[使用例](../Examples/sliding_window_aggregation_example.cpp)を参照してください。

以下、Nは要素数です。値の演算・コピーをO(1)として計算量を記します。

## コンストラクタ

```text
explicit SlidingWindowAggregation(const T& identity, Op op = Op{});
SlidingWindowAggregation(const SlidingWindowAggregation&) = default;
SlidingWindowAggregation(SlidingWindowAggregation&& other);
```

指定した単位元identity・結合演算opで、空のキューを構築します。

**制約:** 冒頭の値型・演算の前提を満たすこと。指定する列長は内部容量の上限内です。

**計算量:** 空の構築・ムーブ構築はO(1)、コピーは保持要素数に比例。

## push_back

```text
void push_back(const T& value);
```

列の末尾へ値を追加します。

**制約:** 共通の前提を満たすこと。

**計算量:** 償却O(1)、1回の最悪O(N)。値のコピー・演算をO(1)とします。

## pop_front

```text
void pop_front();
```

列の先頭を削除します。

**制約:** 空でないこと。

**計算量:** 償却O(1)、1回の最悪O(N)。値のコピー・演算をO(1)とします。

## front

```text
const T& front() const;
```

先頭要素のconst参照を返します。

**制約:** 空でないこと。

**計算量:** O(1)。

## back

```text
const T& back() const;
```

末尾要素のconst参照を返します。

**制約:** 空でないこと。

**計算量:** O(1)。

## all_prod

```text
T all_prod() const;
```

先頭から順にOpで結合した値を返します。空ならidentityです。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## size

```text
std::size_t size() const noexcept;
```

現在のキューの要素数を返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## empty

```text
bool empty() const noexcept;
```

空ならtrueを返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## clear

```text
void clear() noexcept;
```

全要素を削除します。確保容量は保持する場合があります。

**制約:** 共通の前提を満たすこと。

**計算量:** O(N)。

## swap

```text
void swap(SlidingWindowAggregation& other);
```

内容と設定を交換します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。値型・比較器のswapの費用は別です。

```cpp
SlidingWindowAggregation<std::int64_t,std::plus<std::int64_t>> queue(0);
queue.push_back(2); queue.push_back(5);
auto sum = queue.all_prod(); // 7。
queue.pop_front();
auto first = queue.front(); // 5。
```

## 前提と所有権

挿入側と削除側の2つのスタックは、それぞれの順序で求めた集約値を保持します。この集約順によって、非可換の演算でも元の列の順序を保ちます。計算量は演算と値のコピーをO(1)としたものです。文字列連結などの費用は別途かかります。保持領域は過去の最大要素数に比例し、削除やclearの後も容量を保持する場合があります。

front・back・pop_frontは非空が前提です。既定で空を検査してout_of_rangeを投げ、`SKIP_RUNTIME_CHECKS`でこの検査を除きます。front・backの参照は更新・交換・移動で無効になり得ます。T=boolも使えます。

T・Opはコピー可能で、破棄が例外を投げない型を使います。Opに外部副作用を持たせず、コールバックから同じキューを変更しないでください。push・popでコピー・演算・確保が失敗しても列を保ちます。演算の外部状態は巻き戻しません。

コピーは独立、移動元は空です。代入・swapにはT・Opが交換可能であることが必要です。代入の強い例外保証は、その交換がnoexceptの場合に成立します。交換自体が例外を投げる型では途中状態を保証しません。

構成と問題選定は[NyaanのSWAG](https://nyaannyaan.github.io/library/data-structure/slide-window-aggregation.hpp)を参考にし、実装は独立に組み立てています。

## operator=

```text
SlidingWindowAggregation& operator=(const SlidingWindowAggregation& other);
SlidingWindowAggregation& operator=(SlidingWindowAggregation&& other);
```

コピーまたはムーブで内容を置き換え、自身への参照を返します。コピーは独立した内容を持ちます。

**制約:** 型・所有・例外保証は共通の説明に従います。

**計算量:** 保持する配列・ノードのコピーと旧内容の破棄に比例。ムーブ構築とは異なり、代入では旧内容の解放費用も含みます。
