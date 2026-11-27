# 両端キューの集約

先頭と末尾のどちらからでも要素を出し入れし、現在の列の積を求めます。関数合成のように、演算する順番で結果が変わる場合にも使えます。

`op(a,b)` は結合的で、`identity` は左右の単位元です。`T` と `Op` はコピー可能とし、`op` は `const` で呼べる副作用のない演算を使います。要素数を $n$ とし、演算と値のコピーを定数時間と数えます。

## `DequeAggregation`

```text
template <class T, class Op>
class DequeAggregation;

explicit DequeAggregation(const T &identity, Op op = Op{});
```

空の列を $O(1)$ 時間・領域で作ります。コピーすると列を独立して所有し、移動元は空になります。過去の最大要素数を $M$ とすると、保持する領域は $O(M+1)$ です。

## `push_front`

```text
void push_front(const T &value);
```

先頭に値を1つ追加します。償却 $O(1)$ です。

## `push_back`

```text
void push_back(const T &value);
```

末尾に値を1つ追加します。償却 $O(1)$ です。

## `pop_front`

```text
void pop_front();
```

空でない列から先頭を削除します。償却 $O(1)$、1回の最悪は $O(n)$ です。

## `pop_back`

```text
void pop_back();
```

空でない列から末尾を削除します。償却 $O(1)$、1回の最悪は $O(n)$ です。

## `front`

```text
const T &front() const;
```

空でない列の先頭を読み取り専用参照で返します。$O(1)$ です。

## `back`

```text
const T &back() const;
```

空でない列の末尾を読み取り専用参照で返します。$O(1)$ です。

## `all_prod`

```text
T all_prod() const;
```

先頭から末尾の順に集約した値を返します。空の列では単位元を返します。$O(1)$ です。

## `size`

```text
std::size_t size() const noexcept;
```

要素数を $O(1)$ で返します。

## `empty`

```text
bool empty() const noexcept;
```

空の列なら `true` を返します。$O(1)$ です。

## `clear`

```text
void clear() noexcept;
```

全要素を削除します。時間は $O(n)$ です。

## `swap`

```text
void swap(DequeAggregation &other) noexcept
    requires std::is_nothrow_swappable_v<T> && std::is_nothrow_swappable_v<Op>;
```

列・演算・単位元を $O(1)$ で交換します。代入と交換には `T` と `Op` を例外なしで交換できることが必要です。

入力検査が有効な場合、空の列の両端を取得・削除すると `std::out_of_range` を送出します。

両端の参照は更新・交換・移動で無効になる場合があります。追加・削除中に確保、値のコピー、演算が失敗しても元の列は保持されます。

## 使用例

```cpp
sirkit::DequeAggregation functions(Affine{1, 0}, Compose{});
functions.push_back({2, 0});  // x → 2x
functions.push_front({1, 3}); // x → x+3
const auto both = functions.all_prod(); // x → 2x+6
functions.pop_back();
const auto first = functions.all_prod(); // x → x+3
```

[Sliding Window Aggregation](sliding_window_aggregation.md) は末尾への追加と先頭の削除に使えます。

[使用例](../Examples/deque_aggregation_example.cpp) ／ [Verify](../Verify/current/submissions/deque_operate_all_composite.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/deque_operate_all_composite)
