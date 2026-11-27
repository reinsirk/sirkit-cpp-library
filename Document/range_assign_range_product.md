# 区間代入・区間積

区間の全要素を同じ値に置き換え、現在の順番で区間積を求めます。関数合成のように、順番によって結果が変わる演算にも使えます。

`op(left,right)` は結合的な演算、`e()` は単位元です。`op` と `S` のコピー・ムーブの構築・代入には `noexcept` が必要です。演算の途中の値も `S` で表せるものを使います。値の等値比較は不要です。計算量では演算と値のコピーを定数時間と数えます。

添字は0始まりで、長さ $n$ と端点は `std::uint64_t` です。入力検査が有効な場合、範囲外の位置や区間には `std::out_of_range` を送出します。

## `RangeAssignRangeProduct`

```text
template <class S, auto op, auto e>
class RangeAssignRangeProduct;

explicit RangeAssignRangeProduct(std::uint64_t n = 0);
RangeAssignRangeProduct(std::uint64_t n, const S &initial);
explicit RangeAssignRangeProduct(std::span<const S> values);
```

全要素を単位元で初期化する場合は $O(1)$、一様な値 `initial` を指定する場合は $O(1+\log(n+1))$ 時間です。一様な配列の構築領域は $O(1)$。初期列を渡す場合は $O(n+1)$ 時間・領域です。

## コピー・移動

```text
RangeAssignRangeProduct(const RangeAssignRangeProduct &other);
RangeAssignRangeProduct(RangeAssignRangeProduct &&other) noexcept;
RangeAssignRangeProduct &operator=(RangeAssignRangeProduct other) noexcept;
```

コピーは独立で、保存領域の大きさに比例します。移動による構築は $O(1)$ で、移動元は長さ0になります。代入では、これに代入前の領域を破棄する時間が加わります。

## `assign`

```text
void assign(std::uint64_t left, std::uint64_t right, const S &value);
```

半開区間 `[left,right)` の各要素を `value` に置き換えます。`left <= right <= n` を満たしてください。空区間では値は変化しません。

各区間を1つの節点で持ち、分割した両端と代入した区間の積を計算します。代入部分に含まれた節点を回収し、以後の代入で再利用します。初期列の構築と $q$ 回の代入の合計は $O(1+n+q(1+\log(n+1)))$ 時間です。一様な初期値から始める場合、その初期化を除く代入の時間は償却 $O(1+\log(n+1))$ です。

## `prod`

```text
S prod(std::uint64_t left, std::uint64_t right) const;
```

`left <= right <= n` を満たす半開区間の値を左から右へ集約します。空区間は単位元です。$O(1+\log(n+1))$ です。

## `get`

```text
S get(std::uint64_t position) const;
```

`position < n` の位置の現在値を返します。$O(1+\log(n+1))$ です。

## `all_prod`

```text
S all_prod() const noexcept;
```

列全体の積を $O(1)$ で返します。

## `size`

```text
std::uint64_t size() const noexcept;
```

配列長を $O(1)$ で返します。

## `stored_ranges`

```text
std::size_t stored_ranges() const noexcept;
```

内部で現在保存している区間数を $O(1)$ で返します。隣接区間の値を比較しないため、同じ値の区間は分かれていることがあります。同時に保存した最大区間数を $P$ とすると、更新後の領域は $O(P+1)$ です。節点を最大 $2^{32}-1$ 個まで保存でき、処理系の配列容量またはこの個数を超えると `std::length_error` を送出します。

## 使用例

```cpp
const std::vector<Affine> values{{1, 1}, {2, 0}, {1, 4}};
sirkit::RangeAssignRangeProduct<Affine, Compose, Identity> data(values);
const auto before = data.prod(0, 3); // x → 2x+6
data.assign(1, 3, {3, 0});
const auto after = data.prod(0, 3);  // x → 9x+9
```

[遅延 Segment Tree](lazy_segment_tree.md) では、値の置き換え以外の作用も定義できます。

[使用例](../Examples/range_assign_range_product_example.cpp) ／ [Verify](../Verify/current/submissions/range_set_range_composite.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/range_set_range_composite)
