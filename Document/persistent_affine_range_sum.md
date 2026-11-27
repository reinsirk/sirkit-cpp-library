# 永続の区間アフィン更新・和

区間を更新した後も、過去の配列を参照できます。各操作の結果を保存し、そこから新しい版を作れます。

添字は0始まりです。区間は `[left,right)` とし、`0 <= left <= right <= size()` を満たしてください。法 `Mod` は正の符号なし32ビット整数で、既定値は998244353です。

`Root` は版を表す非所有の値型です。木をコピーすると節点の保存領域を共有し、同じ版をどちらにも渡せます。`initial_root()` または更新・コピーの戻り値を保存し、一つの版から何度でも分岐できます。保存領域を所有する木が残る間は版が有効です。並行して更新・照会する場合は外部で同期します。

## `PersistentAffineRangeSum`

```text
template <std::uint32_t Mod = 998244353>
class PersistentAffineRangeSum;

explicit PersistentAffineRangeSum(std::span<const std::int64_t> values = {});
```

初期列から木を作ります。要素数 $n$ は `INT_MAX` 以下です。 $O(n+1)$ 時間・領域です。

## `initial_root`

```text
Root initial_root() const noexcept;
```

最初の版を返します。 $O(1)$ です。

## `range_affine`

```text
Root range_affine(Root from, int left, int right, std::int64_t a, std::int64_t b);
```

`from` の区間に `x → a*x+b` を適用した版を返します。同じ箇所への変換は適用した順に合成されます。元の版と区間外の値は保ちます。 償却 $O(1+\log(n+1))$ 時間・追加節点です。全区間・空区間の操作は $O(1)$ 時間で節点を追加しません。

## `range_copy`

```text
Root range_copy(Root dst, Root src, int left, int right);
```

`dst` の区間を `src` の同じ位置の値に置き換えた版を返します。元の両版と区間外の値は保ちます。 償却 $O(1+\log(n+1))$ 時間・追加節点です。全区間・空区間の操作は $O(1)$ 時間で節点を追加しません。

## `range_sum`

```text
std::uint32_t range_sum(Root root, int left, int right) const;
```

指定版の区間和の非負剰余を返します。空区間の和は0です。 $O(1+\log(n+1))$ 時間で節点を追加しません。

## `all_sum`

```text
std::uint32_t all_sum(Root root) const;
```

指定版の全要素の和の非負剰余を返します。 $O(1)$ です。

## `size`

```text
int size() const noexcept;
```

各版に共通する要素数を返します。 $O(1)$ です。

## `allocated_nodes`

```text
std::size_t allocated_nodes() const noexcept;
```

共有領域に保存した節点の数を返します。保存節点数の上限は $2^{32}-1$ 個です。 $O(1)$ です。

## 使用例

```cpp
const std::vector<std::int64_t> values{1, 2, 3};
sirkit::PersistentAffineRangeSum<> tree(values);
const auto initial = tree.initial_root();
const auto changed = tree.range_affine(initial, 0, 3, 2, 1); // {3, 5, 7}
const auto mixed = tree.range_copy(changed, initial, 1, 2);  // {3, 2, 7}
assert(tree.all_sum(mixed) == 12);
```

[永続セグメント木](persistent_segment_tree.md) は点更新と一般のモノイド積を扱います。

[使用例](../Examples/persistent_affine_range_sum_example.cpp) ／ [Verify](../Verify/current/submissions/persistent_range_affine_range_sum.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/persistent_range_affine_range_sum)
