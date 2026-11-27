# 点代入・区間整列・区間積

キーと値をペアで持つ配列です。キーで区間を並べ替え、現在の順番で値を集約します。順番で結果が変わる関数合成にも使えます。

要素 `Item` は `std::int32_t key` と `S value` を持ちます。`op(left,right)` は結合的とし、`e()` は単位元を返します。`op` は `noexcept` とし、`S` のコピーとムーブの構築・代入が例外を送出しない型が必要です。添字は0始まりです。添字 `index` は `0 <= index < size()`、半開区間 `[left,right)` は `0 <= left <= right <= size()` を満たしてください。計算量では値の演算を定数時間とし、$W=64$ はキーと内部IDを詰めたビット幅です。

## `RangeSortAggregation`

```text
template <class S, auto op, auto e>
class RangeSortAggregation;

explicit RangeSortAggregation(std::span<const Item> items = {});
```

キーと値の列を渡します。要素数 $n$ は `INT_MAX` 以下です。 $O(n+1)$ 時間・領域です。

## `sort`

```text
void sort(int left, int right, bool descending = false);
```

`[left,right)` を整列します。既定はキーの昇順、`true` は降順です。同じキーの要素は構築時の位置を表す内部ID順に同じ方向で並びます。 構築と、整列を含む $q$ 操作の合計で $O(1+(n+q)(W+\log(n+1)))$ 時間です。

## `set`

```text
void set(int index, std::int32_t key, S value);
```

指定位置のキーと値を置き換えます。要素の内部IDは保ちます。 $O(W+\log(n+1))$ です。

## `get`

```text
Item get(int index) const;
```

指定位置の現在のキーと値を返します。 $O(W+\log(n+1))$ です。

## `prod`

```text
S prod(int left, int right) const;
```

`[left,right)` の値を現在の並びの左から右へ集約します。空区間では単位元を返します。 $O(W+\log(n+1))$ です。

## `all_prod`

```text
S all_prod() const noexcept;
```

列全体を現在の並びで集約した値を返します。 $O(1)$ です。

## `size`

```text
int size() const noexcept;
```

要素数を返します。 $O(1)$ です。

## 使用例

```cpp
using Data = sirkit::RangeSortAggregation<Affine, compose, identity>;
const std::vector<Data::Item> items{{3, {2, 0}}, {1, {1, 1}}};
Data data(items);
const auto before = data.prod(0, 2); // x -> 2x+1
data.sort(0, 2);
const auto after = data.prod(0, 2);  // x -> 2x+2
```

[Segment Tree](segment_tree.md) は固定順での集約に、[暗黙 Treap](implicit_treap.md) は挿入や移動を伴う可変長列に使えます。

[使用例](../Examples/range_sort_aggregation_example.cpp) ／ [Verify](../Verify/current/submissions/point_set_range_sort_range_composite.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/point_set_range_sort_range_composite)
